#ifndef _TUTTLE_PLUGIN_DEBUGIMAGEEFFECTAPI_PROCESS_HPP_
#define _TUTTLE_PLUGIN_DEBUGIMAGEEFFECTAPI_PROCESS_HPP_

#include "DebugImageEffectApiPlugin.hpp"

#include <tuttle/plugin/image/gil/globals.hpp>
#include <tuttle/plugin/ImageGilFilterProcessor.hpp>
#include <tuttle/plugin/exceptions.hpp>

#include <ofxsImageEffect.h>
#include <ofxsMultiThread.h>

#include <boost/scoped_ptr.hpp>

#include <cstdlib>
#include <cassert>
#include <cmath>
#include <vector>
#include <iostream>

namespace tuttle {
namespace plugin {
namespace debugImageEffectApi {

/**
 * @brief DebugImageEffectApi process
 *
 */
template<class View>
class DebugImageEffectApiProcess : public ImageGilFilterProcessor<View>
{
protected:
	DebugImageEffectApiPlugin&    _plugin;        ///< Rendering plugin

public:
	DebugImageEffectApiProcess( DebugImageEffectApiPlugin& instance );

	void setup( const OFX::RenderArguments& args );

	// Do some processing
	void multiThreadProcessImages( const OfxRectI& procWindowRoW );
};

}
}
}

#include "DebugImageEffectApiProcess.tcc"

#endif
