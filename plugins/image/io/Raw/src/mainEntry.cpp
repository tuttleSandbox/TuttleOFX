#define OFXPLUGIN_VERSION_MAJOR 0
#define OFXPLUGIN_VERSION_MINOR 0

#include <tuttle/plugin/Plugin.hpp>
#include "reader/RawReaderPluginFactory.hpp"
//#include "writer/RawWriterPluginFactory.hpp"

namespace OFX {
namespace Plugin {

void getPluginIDs( OFX::PluginFactoryArray& ids )
{
	mAppendPluginFactory( ids, tuttle::plugin::raw::reader::RawReaderPluginFactory, "fr.tuttle.rawreader" );
	//mAppendPluginFactory( ids, tuttle::plugin::png::writer::RAWWriterPluginFactory, "fr.tuttle.rawwriter" );
}

}
}
