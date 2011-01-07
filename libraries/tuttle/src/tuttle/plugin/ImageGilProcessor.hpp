#ifndef _TUTTLE_PLUGIN_IMAGEGILPROCESSOR_HPP_
#define _TUTTLE_PLUGIN_IMAGEGILPROCESSOR_HPP_

#include "exceptions.hpp"
#include "OfxProgress.hpp"

#include <tuttle/plugin/image/gil/globals.hpp>
#include <tuttle/plugin/exceptions.hpp>
#include <tuttle/common/math/rectOp.hpp>

#include <ofxsImageEffect.h>
#include <ofxsMultiThread.h>
#include <ofxsUtilities.h>

#include <boost/scoped_ptr.hpp>
#include <boost/exception/info.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/throw_exception.hpp>

#include <cstdlib>
#include <vector>

namespace tuttle {
namespace plugin {

template<class DView>
DView getView( OFX::Image* img, const OfxRectI& rod );

/**
 * @brief Base class that can be used to process images of any type using boost::gil library view to access images.
 */
template <class DView>
class ImageGilProcessor : public OFX::MultiThread::Processor
	, public tuttle::plugin::OfxProgress
{
public:
	typedef typename DView::value_type DPixel;
	typedef typename image_from_view<DView>::type DImage;

private:
	unsigned int _nbThreads;

protected:
	OFX::ImageEffect& _effect; ///< effect to render with
	OFX::RenderArguments _renderArgs; ///< render arguments
	OFX::Clip* _clipDst;       ///< Destination image clip
	boost::scoped_ptr<OFX::Image> _dst;
	OfxRectI _dstPixelRod;
	OfxPointI _dstPixelRodSize;
	DView _dstView; ///< image to process into

public:
	/** @brief ctor */
	ImageGilProcessor( OFX::ImageEffect& effect );
	virtual ~ImageGilProcessor();

	void setNoMultiThreading()                        { _nbThreads = 1; }
	void setNbThreads( const unsigned int nbThreads ) { _nbThreads = nbThreads; }
	void setNbThreadsAuto()                           { _nbThreads = 0; }

	/** @brief called before any MP is done */
	virtual void preProcess() { progressBegin( _renderArgs.renderWindow.y2 - _renderArgs.renderWindow.y1 ); }

	/** @brief called before any MP is done */
	virtual void postProcess() { progressEnd(); }

	virtual void setup( const OFX::RenderArguments& args )
	{
		// destination view
		_dst.reset( _clipDst->fetchImage( args.time ) );
		if( !_dst.get() )
			BOOST_THROW_EXCEPTION( exception::ImageNotReady() );
		if( _dst->getRowBytes() <= 0 )
			BOOST_THROW_EXCEPTION( exception::WrongRowBytes() );
		//		_dstPixelRod = _dst->getRegionOfDefinition(); // bug in nuke, returns bounds
		_dstPixelRod       = _clipDst->getPixelRod( args.time, args.renderScale );
		_dstPixelRodSize.x = ( this->_dstPixelRod.x2 - this->_dstPixelRod.x1 );
		_dstPixelRodSize.y = ( this->_dstPixelRod.y2 - this->_dstPixelRod.y1 );
		_dstView           = getView( _dst.get(), _dstPixelRod );

		#ifndef TUTTLE_PRODUCTION
		// init dst buffer with red to highlight uninitialized pixels
		const OfxRectI dstBounds = this->translateRoWToOutputClipCoordinates( _dst->getBounds() );
		DView dstToFill           = boost::gil::subimage_view( _dstView,
		                                                      dstBounds.x1, dstBounds.y1,
		                                                      dstBounds.x2 - dstBounds.x1, dstBounds.y2 - dstBounds.y1 );
		const boost::gil::rgba32f_pixel_t errorColor( 1.0, 0.0, 0.0, 1.0 );
		fill_pixels( dstToFill, errorColor );
		#endif
	}

	/** @brief fetch output and inputs clips */
	virtual void setupAndProcess( const OFX::RenderArguments& args )
	{
		_renderArgs = args;
		try
		{
			setup( args );
		}
		catch( exception::Common& e )
		{
			progressEnd();

			// throw an error, only if the host is not trying to abort the rendering
			// else return without error
			if( ! _effect.abort() )
			{
				throw;
			}
		}
		catch(...)
		{
			progressEnd();

			// throw an error, only if the host is not trying to abort the rendering
			// else return without error
			if( ! _effect.abort() )
			{
				throw;
			}
		}

		// Call the base class process member
		this->process();
	}

	/**
	 * @brief Return a full gil view of an image.
	 */
	DView getView( OFX::Image* img, const OfxRectI& rod ) const
	{
		return tuttle::plugin::getView<DView>( img, rod );
	}

	/** @brief overridden from OFX::MultiThread::Processor. This function is called once on each SMP thread by the base class */
	void multiThreadFunction( unsigned int threadId, unsigned int nThreads )
	{
		// slice the y range into the number of threads it has
		int dy   = std::abs( _renderArgs.renderWindow.y2 - _renderArgs.renderWindow.y1 );
		int y1   = _renderArgs.renderWindow.y1 + threadId * dy / nThreads;
		int step = ( threadId + 1 ) * dy / nThreads;
		int y2   = _renderArgs.renderWindow.y1 + ( step < dy ? step : dy );

		OfxRectI winRoW = _renderArgs.renderWindow;

		winRoW.y1 = y1;
		winRoW.y2 = y2;

		// and render that thread on each
		multiThreadProcessImages( winRoW );
	}

	/** @brief this is called by multiThreadFunction to actually process images, override in derived classes */
	virtual void multiThreadProcessImages( const OfxRectI& windowRoW ) = 0;

	// to output clip coordinates
	OfxRectI translateRoWToOutputClipCoordinates( const OfxRectI& windowRoW ) const
	{
		return translateRegion( windowRoW, _dstPixelRod );
	}

	/** @brief called to process everything */
	virtual void process();
};

template <class DView>
ImageGilProcessor<DView>::ImageGilProcessor( OFX::ImageEffect& effect )
	: OfxProgress( effect )
	, _nbThreads( 0 )
	,                 // auto, maximum allowable number of CPUs will be used
	_effect( effect )
{
	_renderArgs.renderWindow.x1 = _renderArgs.renderWindow.y1 = _renderArgs.renderWindow.x2 = _renderArgs.renderWindow.y2 = 0;
	_renderArgs.renderScale.x   = _renderArgs.renderScale.y = 0;
	_renderArgs.time            = -1;
	_renderArgs.fieldToRender   = OFX::eFieldNone;

	_clipDst = effect.fetchClip( kOfxImageEffectOutputClipName );
}

template <class DView>
ImageGilProcessor<DView>::~ImageGilProcessor()
{}

template <class DView>
void ImageGilProcessor<DView>::process( void )
{
	// is it OK ?
	if( _renderArgs.renderWindow.x2 - _renderArgs.renderWindow.x1 == 0 ||
	    _renderArgs.renderWindow.y2 - _renderArgs.renderWindow.y1 == 0 )
	{
		BOOST_THROW_EXCEPTION( exception::ImageFormat() << exception::user( "RenderWindow empty !" ) );
	}
	// call the pre MP pass
	preProcess();

	// call the base multi threading code, should put a pre & post thread calls in too
	multiThread( _nbThreads );

	// call the post MP pass
	postProcess();
}

/**
 * @brief Return a full gil view of an image.
 *        If we only have a tiled image, we use subimage_view to fit to the rod.
 * @param img the ofx image object
 * @param rod normally we don't need this parameter because we can get it from the image,
 *            but there is a bug in nuke (which return the bounds),
 *            so we need to use the rod of the clip and not from the image.
 */
template<class DView>
DView getView( OFX::Image* img, const OfxRectI& rod )
{
	using namespace boost::gil;
	typedef typename DView::value_type DPixel;

	//	OfxRectI imgrod = img->getRegionOfDefinition(); // bug in nuke returns bounds... not the clip rod with renderscale...
	OfxRectI bounds = img->getBounds();
	//	TUTTLE_COUT_VAR( bounds );
	//	TUTTLE_COUT_VAR( imgrod );
	//	TUTTLE_COUT_VAR( rod );
	point2<int> tileSize = point2<int>( bounds.x2 - bounds.x1,
	                                    bounds.y2 - bounds.y1 );

	// Build views
	DView tileView = interleaved_view( tileSize.x, tileSize.y,
	                                  static_cast<DPixel*>( img->getPixelData() ),
	                                  img->getRowBytes() );

	// if the tile is equals to the full image
	// directly return the tile
	if( bounds.x1 == rod.x1 && bounds.y1 == rod.y1 &&
	    bounds.x2 == rod.x2 && bounds.y2 == rod.y2 )
		return tileView;

	// view the tile as a full image
	return subimage_view( tileView, rod.x1 - bounds.x1, rod.y1 - bounds.y1, rod.x2 - rod.x1, rod.y2 - rod.y1 );
}

}
}

#endif

