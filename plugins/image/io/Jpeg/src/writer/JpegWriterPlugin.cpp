#include "JpegWriterDefinitions.hpp"
#include "JpegWriterPlugin.hpp"
#include "JpegWriterProcess.hpp"

#include <boost/gil/gil_all.hpp>

namespace tuttle {
namespace plugin {
namespace jpeg {
namespace writer {

JpegWriterPlugin::JpegWriterPlugin( OfxImageEffectHandle handle )
	: WriterPlugin( handle )
{
	_premult = fetchBooleanParam( kParamPremult );
	_quality = fetchIntParam( kParamQuality );
}

JpegWriterProcessParams JpegWriterPlugin::getProcessParams( const OfxTime time )
{
	JpegWriterProcessParams params;

	params._filepath = getAbsoluteFilenameAt( time );
	params._quality  = this->_quality->getValue();
	params._premult  = this->_premult->getValue();
	return params;
}

/**
 * @brief The overridden render function
 * @param[in]   args     Rendering parameters
 */
void JpegWriterPlugin::render( const OFX::RenderArguments& args )
{
	WriterPlugin::render( args );

	doGilRender<JpegWriterProcess>( *this, args );
}

}
}
}
}
