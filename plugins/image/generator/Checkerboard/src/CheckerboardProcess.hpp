#ifndef _TUTTLE_PLUGIN_CHECKERBOARD_PROCESS_HPP_
#define _TUTTLE_PLUGIN_CHECKERBOARD_PROCESS_HPP_

#include "CheckerboardAlgorithm.hpp"

#include <tuttle/plugin/image/gil/globals.hpp>
#include <tuttle/plugin/ImageGilProcessor.hpp>
#include <tuttle/plugin/exceptions.hpp>

#include <cstdlib>
#include <cassert>
#include <cmath>
#include <vector>
#include <iostream>
#include <ofxsImageEffect.h>
#include <ofxsMultiThread.h>

#include <boost/gil/gil_all.hpp>
#include <boost/scoped_ptr.hpp>

namespace tuttle {
namespace plugin {
namespace checkerboard {

template<class View>
struct CheckerboardParams
{
	typedef typename View::value_type Pixel;
	boost::gil::point2<int> _boxes;
	Pixel _color1;
	Pixel _color2;
};

template<class View>
class CheckerboardProcess : public ImageGilProcessor<View>
{
public:
	typedef typename View::value_type Pixel;
	typedef CheckerboardFunctor<Pixel> CheckerboardFunctorT;
	typedef typename CheckerboardFunctorT::point_t Point;
	typedef boost::gil::virtual_2d_locator<CheckerboardFunctorT, false> Locator;
	typedef boost::gil::image_view<Locator> CheckerboardVirtualView;

protected:
	CheckerboardPlugin&    _plugin;        ///< Rendering plugin
	CheckerboardVirtualView _srcView;       ///< Source view

public:
	CheckerboardProcess( CheckerboardPlugin& instance );

	void setup( const OFX::RenderArguments& args );

	CheckerboardParams<View> getParams();

	void multiThreadProcessImages( const OfxRectI& procWindowRoW );
};

}
}
}

#include "CheckerboardProcess.tcc"

#endif
