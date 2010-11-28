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
		Graph::Node& source     = g.createNode( "fr.tuttle.jpegreader" );
		Graph::Node& cadre      = g.createNode( "fr.tuttle.jpegreader" );
		Graph::Node& background = g.createNode( "fr.tuttle.jpegreader" );
		Graph::Node& sourceBd = g.createNode( "fr.tuttle.bitdepth" );
		Graph::Node& cadreBd = g.createNode( "fr.tuttle.bitdepth" );
		Graph::Node& backgroundBd = g.createNode( "fr.tuttle.bitdepth" );
		Graph::Node& colorSuppr = g.createNode( "fr.tuttle.duranduboi.colorsuppress" );
		Graph::Node& invertAlpha  = g.createNode( "fr.tuttle.invert" );
		Graph::Node& merge  = g.createNode( "fr.tuttle.merge" );
		Graph::Node& write    = g.createNode( "fr.tuttle.jpegwriter" );

		COUT( "__________________________________________________2" );
		// Setup parameters
		source.getParam( "filename" ).set( "data/key/source.jpg" );
		cadre.getParam( "filename" ).set( "data/key/cadre.jpg" );
		background.getParam( "filename" ).set( "data/key/background.jpg" );

		sourceBd.getParam( "outputBitDepth" ).set( 3 );
		cadreBd.getParam( "outputBitDepth" ).set( 3 );
		backgroundBd.getParam( "outputBitDepth" ).set( 3 );
		colorSuppr.getParam( "greenRate" ).set( 1.0 );
		colorSuppr.getParam( "output" ).set( 2 );
		invertAlpha.getParam( "processR" ).set( false );
		invertAlpha.getParam( "processG" ).set( false );
		invertAlpha.getParam( "processB" ).set( false );
		invertAlpha.getParam( "processA" ).set( true );
		merge.getParam( "mergingFunction" ).set( 19 );
		write.getParam( "premult" ).set( false );
		write.getParam( "filename" ).set( "data/key/output.jpg" );

		COUT( "__________________________________________________3" );
		g.connect( source, sourceBd );
		g.connect( cadre, cadreBd );
		g.connect( background, backgroundBd );

		g.connect( sourceBd, colorSuppr );
		g.connect( colorSuppr, invertAlpha );
		g.connect( invertAlpha, merge.getClip("A") );
		g.connect( backgroundBd, merge.getClip("B") );
		g.connect( merge, write );

		COUT( "__________________________________________________4" );
//		std::list<std::string> outputs;
//		outputs.push_back( write.getName() );
//		g.compute( outputs, 0, 0 );
		g.compute( write, 0 );
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

