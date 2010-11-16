#include <tuttle/host/Graph.hpp>

void sam_terminate( void )
{
	std::cerr << "Sorry, Sam has encountered a fatal error." << std::endl;
	std::cerr << "Please report this bug." << std::endl;
	exit( -1 );
}

void sam_unexpected( void )
{
	std::cerr << "Sorry, Sam has encountered an unexpected exception." << std::endl;
	std::cerr << "Please report this bug." << std::endl;
	BOOST_THROW_EXCEPTION( std::runtime_error( "Sorry, Sam has encountered an unexpected exception.\nPlease report this bug." ) );
}

int main( int argc, char** argv )
{
	std::set_terminate( &sam_terminate );
	std::set_unexpected( &sam_unexpected );
	try
	{
		using namespace tuttle::host;
		COUT( "__________________________________________________0" );
		// Core::instance().getPluginCache().addDirectoryToPath( "/path/to/plugins" );
		// Core::instance().getPluginCache().scanPluginFiles();
		Core::instance().preload();

		COUT( Core::instance().getImageEffectPluginCache() );

		COUT( "__________________________________________________1" );

		Graph g;
		Graph::Node& read1    = g.createNode( "fr.tuttle.jpegreader" );
		Graph::Node& bitdepth = g.createNode( "fr.tuttle.bitdepth" );
		Graph::Node& invert1  = g.createNode( "fr.tuttle.invert" );
		Graph::Node& write1    = g.createNode( "fr.tuttle.jpegwriter" );

		COUT( "__________________________________________________2" );
		// Setup parameters
		read1.getParam( "filename" ).set( "data/input.jpg" );
		bitdepth.getParam( "outputBitDepth" ).set( 3 );
		COUT_VAR( bitdepth.getParam( "outputBitDepth" ).getString() );
		write1.getParam( "filename" ).set( "data/output1.jpg" );

		COUT( "__________________________________________________3" );
		g.connect( read1, bitdepth );
		g.connect( bitdepth, invert1 );
		g.connect( invert1, write1 );

		COUT( "__________________________________________________4" );
		std::list<std::string> outputs;
		outputs.push_back( write1.getName() );
		g.compute( outputs, 0, 0 );
	}
	catch( tuttle::exception::Common& e )
	{
		std::cout << "Tuttle Exception : main de sam." << std::endl;
		std::cerr << boost::diagnostic_information( e );
	}
	catch(... )
	{
		std::cerr << "Exception ... : main de sam." << std::endl;
		std::cerr << boost::current_exception_diagnostic_information();

	}

	return 0;
}

