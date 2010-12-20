#include <tuttle/host/Graph.hpp>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <iostream>
#include <fstream>
#include <iterator>


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

// A helper function to simplify the main part. (boost-opations)
template<class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v)
{
    std::copy(v.begin(), v.end(), std::ostream_iterator<T>(std::cout, " ")); 
    return os;
}

int main( int argc, char** argv )
{
	std::set_terminate( &sam_terminate );
	std::set_unexpected( &sam_unexpected );
	try
	{
        float suppR, suppG, suppB, normalizeParam1;
        std::string config_file = "data/key/config.ini";
        std::string src, bg, frame;
    
        // Declare a group of options that will be 
        // allowed only on command line
        po::options_description generic("Generic options");
        generic.add_options()
            ("help", "produce help message")
            ;
    
        // Declare a group of options that will be 
        // allowed both on command line and in
        // config file
        po::options_description config("Configuration");
        config.add_options()
            ("sppress-R,r", po::value<float>(&suppR)->default_value(0.0f), 
                  "red suppression level [0.0;1.0]")
            ("sppress-G,g", po::value<float>(&suppG)->default_value(1.0f), 
                  "green suppression level [0.0;1.0]")
            ("sppress-B,b", po::value<float>(&suppB)->default_value(0.0f), 
                  "blue suppression level [0.0;1.0]")
            ("normalizeParam1,np1", po::value<float>(&normalizeParam1)->default_value(0.4f), 
                  "normalizeParam1 [0.0;1.0]?")
            ("src", po::value<std::string>(&src)->default_value("data/key/source.jpg"), 
                  "source image path")
            ("bg", po::value<std::string>(&bg)->default_value("data/key/background.jpg"), 
                  "background image path")
            ("frame", po::value<std::string>(&frame)->default_value("data/key/foreground.jpg"), 
                  "foregroung image path")
            ;

        // Hidden options, will be allowed both on command line and
        // in config file, but will not be shown to the user.
        po::options_description hidden("Hidden options");
        hidden.add_options()
            ("input-file", po::value< std::vector<std::string> >(), "input file")
            ;

        
        po::options_description cmdline_options;
        cmdline_options.add(generic).add(config).add(hidden);

        po::options_description config_file_options;
        config_file_options.add(config).add(hidden);

        po::options_description visible("Allowed options");
        visible.add(generic).add(config);
        
        po::positional_options_description p;
        p.add("input-file", -1);
        
        po::variables_map vm;
        store(po::command_line_parser(argc, argv).
              options(cmdline_options).positional(p).run(), vm);
        notify(vm);
        
        std::ifstream ifs(config_file.c_str());
        if (!ifs)
        {
            std::cout << "can not open config file: " << config_file << "\n";
            return 0;
        }
        else
        {
            store(parse_config_file(ifs, config_file_options), vm);
            notify(vm);
        }
    
        if (vm.count("help")) {
            std::cout << visible << "\n";
            return 0;
        }

        if (vm.count("input-file"))
        {
            std::cout << "Input files are: " 
                 << vm["input-file"].as< std::vector<std::string> >() << "\n";
        }

        std::cout << "suppress: R=" << suppR << ", G=" << suppG << ", B=" << suppB << "\n";  
		
		
		using namespace tuttle::host;
		COUT( "__________________________________________________0" );
		// Core::instance().getPluginCache().addDirectoryToPath( "/path/to/plugins" );
		// Core::instance().getPluginCache().scanPluginFiles();
		Core::instance().preload();

		COUT( Core::instance().getImageEffectPluginCache() );

		COUT( "__________________________________________________1" );

		/*
		probleme de taille 
		*/
		
		Graph g;
		Graph::Node& source     = g.createNode( "fr.tuttle.jpegreader" );
		Graph::Node& cadre      = g.createNode( "fr.tuttle.jpegreader" );
		Graph::Node& background = g.createNode( "fr.tuttle.jpegreader" );
		Graph::Node& sourceBd = g.createNode( "fr.tuttle.bitdepth" );
		Graph::Node& cadreBd = g.createNode( "fr.tuttle.bitdepth" );
		Graph::Node& backgroundBd = g.createNode( "fr.tuttle.bitdepth" );
		Graph::Node& colorSuppr = g.createNode( "fr.tuttle.duranduboi.colorsuppress" ); // suppression BG
		Graph::Node& invertAlpha  = g.createNode( "fr.tuttle.invert" );
		Graph::Node& normalize = g.createNode( "fr.tuttle.duranduboi.normalize" ); // amelio qualité masque alpha
		Graph::Node& gamma = g.createNode( "fr.tuttle.gamma" ); // amelio qualité masque alpha
		Graph::Node& merge  = g.createNode( "fr.tuttle.merge" );
		Graph::Node& write    = g.createNode( "fr.tuttle.jpegwriter" );
		Graph::Node& writeKey    = g.createNode( "fr.tuttle.jpegwriter" );

		COUT( "__________________________________________________2" );
		// Setup parameters
		source.getParam( "filename" ).set( src );
		cadre.getParam( "filename" ).set( frame );
		background.getParam( "filename" ).set( bg );

		//float param1 = (argc > 1)? atof(argv[1]) : 0.4;
		//float param2 = (argc > 2)? atof(argv[2]) : 1.0;
		//float param3 = (argc > 3)? atof(argv[3]) : 0.0;
		
		// passe de 8 bits à 32 (floats)
		sourceBd.getParam( "outputBitDepth" ).set( 3 );
		cadreBd.getParam( "outputBitDepth" ).set( 3 );
		backgroundBd.getParam( "outputBitDepth" ).set( 3 );

		colorSuppr.getParam( "greenRate" ).set( suppG );
		colorSuppr.getParam( "blueRate" ).set( suppB );
		colorSuppr.getParam( "redRate" ).set( suppR );
		colorSuppr.getParam( "output" ).set( 2 ); // 2: modifie l'image et sort la couche alpha
		//COUT_VAR(invertAlpha);
		invertAlpha.getParam( "processR" ).set( false );
		invertAlpha.getParam( "processG" ).set( false );
		invertAlpha.getParam( "processB" ).set( false );
		invertAlpha.getParam( "processA" ).set( true );
		normalize.getParam( "mode" ).set( 1 );
		normalize.getParam( "srcColorMin" ).setAtIndex( normalizeParam1, 3 ); // 0.2,  3
		normalize.getParam( "processR" ).set( false );
		normalize.getParam( "processG" ).set( false );
		normalize.getParam( "processB" ).set( false );
		normalize.getParam( "processA" ).set( true );
		gamma.getParam( "gammaType" ).set( "RGBA" );
		gamma.getParam( "alpha" ).set( 0.1 );
		merge.getParam( "mergingFunction" ).set( 19 );
		write.getParam( "premult" ).set( false );
		write.getParam( "filename" ).set( "data/key/output.jpg" );
		writeKey.getParam( "premult" ).set( false );
		writeKey.getParam( "filename" ).set( "data/key/outputKey.jpg" );
		
		COUT( "__________________________________________________3" );
		// passage des trois images 8 bits => float
		g.connect( source, sourceBd );
		g.connect( cadre, cadreBd );
		g.connect( background, backgroundBd );

		// suppression de BG 
		g.connect( sourceBd, colorSuppr );
		g.connect( colorSuppr, invertAlpha );
		// amélio Alpha
		g.connect( invertAlpha, gamma /*normalize*/ );

		g.connect( /*normalize*/ gamma, merge.getClip("A") );
		g.connect( backgroundBd, merge.getClip("B") );
		g.connect( merge, write );
		g.connect( invertAlpha, writeKey );

		std::list<std::string> outputs;
		outputs.push_back( write.getName() );
		outputs.push_back( writeKey.getName() );
		COUT( "__________________________________________________4 process" );
		g.compute( outputs, 0 );
//		g.compute( write, 0 );
//
		COUT( "__________________________________________________5" );
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

