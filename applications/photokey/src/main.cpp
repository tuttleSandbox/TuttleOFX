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
		/********************************************************
						ARGUMENTS / CONFIG OPTIONS
		*********************************************************/
		
        float suppR, suppG, suppB, normalizeParam1,paramGamma, resizeW, resizeH, moveX, moveY;
        std::string config_file = "data/key/config.ini";
        std::string src, bg, frame, out;
    
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
            ("gamma", po::value<float>(&paramGamma)->default_value(0.1f), 
                  "gamma [0.0;1.0]?")
            ("resizeW", po::value<float>(&resizeW)->default_value(400.f), 
                  "resize height (px)")
            ("resizeH", po::value<float>(&resizeH)->default_value(300.f), 
                  "resize width (px)")
            ("moveX", po::value<float>(&moveX)->default_value(0.f), 
                  "translate X (px)")
            ("moveY", po::value<float>(&moveY)->default_value(0.f), 
                  "translate X (px)")
            ("src", po::value<std::string>(&src)->default_value("data/key/source.png"), 
                  "source image path")
            ("bg", po::value<std::string>(&bg)->default_value("data/key/background.jpg"), 
                  "background image path")
            ("frame", po::value<std::string>(&frame)->default_value("data/key/cadre.png"), 
                  "foregroung image path")
            ("out", po::value<std::string>(&out)->default_value("data/key/output.jpg"), 
                  "output image path")
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
		
		/********************************************************
							PLUGINS GRAPHE
		*********************************************************/
		
		using namespace tuttle::host;
		TUTTLE_COUT( "__________________________________________________0" );
		// Core::instance().getPluginCache().addDirectoryToPath( "/path/to/plugins" );
		// Core::instance().getPluginCache().scanPluginFiles();
		Core::instance().preload();

		TUTTLE_COUT( Core::instance().getImageEffectPluginCache() );

		TUTTLE_COUT( "__________________________________________________1 Nodes Creation" );
		
		Graph g;
		Graph::Node& source     = g.createNode( "fr.tuttle.imagemagickreader" ); // /!\ imagemagickreader = mirroir haut/bas
		Graph::Node& cadre      = g.createNode( "fr.tuttle.imagemagickreader" );
		Graph::Node& background = g.createNode( "fr.tuttle.imagemagickreader" );
		Graph::Node& sourceBd 	= g.createNode( "fr.tuttle.bitdepth" );
		Graph::Node& cadreBd 	= g.createNode( "fr.tuttle.bitdepth" );
		Graph::Node& backgroundBd=g.createNode( "fr.tuttle.bitdepth" );
		Graph::Node& colorSuppr = g.createNode( "fr.tuttle.duranduboi.colorsuppress" ); // suppression BG
		Graph::Node& invertAlpha= g.createNode( "fr.tuttle.invert" );
		Graph::Node& invertAlpha2=g.createNode( "fr.tuttle.invert" );
		Graph::Node& normalize 	= g.createNode( "fr.tuttle.duranduboi.normalize" ); // amelio qualité masque alpha
		Graph::Node& gamma  	= g.createNode( "fr.tuttle.gamma" ); // amelio qualité masque alpha
		Graph::Node& resize 	= g.createNode( "fr.tuttle.resize" );
		Graph::Node& move2d 	= g.createNode( "fr.tuttle.move2d" );
		Graph::Node& merge  	= g.createNode( "fr.tuttle.merge" );
		Graph::Node& merge2 	= g.createNode( "fr.tuttle.merge" );
		Graph::Node& write  	= g.createNode( "fr.tuttle.jpegwriter" );
		Graph::Node& writeKey	= g.createNode( "fr.tuttle.jpegwriter" );
		Graph::Node& writeMerge1	= g.createNode( "fr.tuttle.jpegwriter" );
		//Graph::Node& writeKey    = g.createNode( "fr.tuttle.imagemagickwriter" ); // aucun effet, setParam doit pas fonctionner

		TUTTLE_COUT( "__________________________________________________2 Param nodes" );
		// Setup parameters
		source.getParam( "filename" ).set( src );
		cadre.getParam( "filename" ).set( frame );
		background.getParam( "filename" ).set( bg );
		
		// passe de 8 bits à 32 (floats)
		sourceBd.getParam( "outputBitDepth" ).set( 3 );
		cadreBd.getParam( "outputBitDepth" ).set( 3 );
		backgroundBd.getParam( "outputBitDepth" ).set( 3 );

		colorSuppr.getParam( "greenRate" ).set( suppG );
		colorSuppr.getParam( "blueRate" ).set( suppB );
		colorSuppr.getParam( "redRate" ).set( suppR );
		colorSuppr.getParam( "output" ).set( 2 ); // 2: modifie l'image et sort la couche alpha
		//TUTTLE_COUT_VAR(invertAlpha);
		invertAlpha.getParam( "processR" ).set( false );
		invertAlpha.getParam( "processG" ).set( false );
		invertAlpha.getParam( "processB" ).set( false );
		invertAlpha.getParam( "processA" ).set( true );
		invertAlpha2.getParam( "processR" ).set( false );
		invertAlpha2.getParam( "processG" ).set( false );
		invertAlpha2.getParam( "processB" ).set( false );
		invertAlpha2.getParam( "processA" ).set( true );
		normalize.getParam( "mode" ).set( 1 );
		normalize.getParam( "srcColorMin" ).setAtIndex( normalizeParam1, 3 ); // 0.2,  3
		normalize.getParam( "processR" ).set( false );
		normalize.getParam( "processG" ).set( false );
		normalize.getParam( "processB" ).set( false );
		normalize.getParam( "processA" ).set( true );
		
		resize.getParam( "size" ).set( resizeW, resizeH );
		move2d.getParam( "translation" ).set( moveX, moveY );
		
		gamma.getParam( "gammaType" ).set( "RGBA" );
		gamma.getParam( "alpha" ).set( paramGamma );
		merge.getParam( "mergingFunction" ).set( 19 );
		//merge.getParam( "rod" ).set( "B" ); // TODO
		merge2.getParam( "mergingFunction" ).set( 19 );
		write.getParam( "premult" ).set( false );
		write.getParam( "filename" ).set( out );
		writeKey.getParam( "premult" ).set( false );
		writeKey.getParam( "filename" ).set( "data/key/outputKey.jpg" );
		writeMerge1.getParam( "premult" ).set( false );
		writeMerge1.getParam( "filename" ).set( "data/key/outputMerge1.jpg" );
		
		TUTTLE_COUT( "__________________________________________________3 Connect nodes" );
		// passage des trois images 8 bits => float
		g.connect( source, sourceBd );
		g.connect( cadre, cadreBd );
		g.connect( background, backgroundBd );

		// suppression de BG 
		g.connect( sourceBd, colorSuppr );
		g.connect( colorSuppr, invertAlpha );
		// amélio Alpha
		g.connect( invertAlpha, gamma );

		std::cout<< "resizeW:" <<resizeW <<" resizeH:" <<resizeH <<" moveX:" <<moveX <<" moveY:" <<moveY <<std::endl;
		if(resizeW<1.f && resizeH < 1.f && moveX < 0.5f && moveY < 0.5f)
		{
			TUTTLE_COUT( "no resize, no move" );
			g.connect( gamma, merge.getClip("A") );
			//g.connect( invertAlpha, merge.getClip("A") ); // test
		}
		else
		{
			TUTTLE_COUT( "resize + move" );
			g.connect( gamma, resize );
			g.connect( resize, move2d );
			
			g.connect( move2d, merge.getClip("A") );
		}
		g.connect( backgroundBd, merge.getClip("B") ); // TODO fix alpha problem merge
		
		g.connect( cadreBd, invertAlpha2 );
		g.connect( invertAlpha2, merge2.getClip("A") );
		g.connect( merge, merge2.getClip("B") );
		
		g.connect( merge2, write );
		g.connect( invertAlpha, writeKey );
		g.connect( merge, writeMerge1 ); // test 

		std::list<std::string> outputs;
		outputs.push_back( write.getName() );
		outputs.push_back( writeKey.getName() );
		outputs.push_back( writeMerge1.getName() );
		TUTTLE_COUT( "__________________________________________________4 process" );
		g.compute( outputs, 0 );
//		g.compute( write, 0 );
//
		TUTTLE_COUT( "__________________________________________________5 terminated :)" );
	}
	catch( tuttle::exception::Common& e )
	{
		std::cerr << "Tuttle Exception : main de photokey." << std::endl;
		std::cerr << boost::diagnostic_information( e );
		return -1;
	}
	catch(... )
	{
		std::cerr << "Exception ... : main de photokey." << std::endl;
		std::cerr << boost::current_exception_diagnostic_information();
		return -2;

	}

	return 0;
}

