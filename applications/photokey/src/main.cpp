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
		
        float suppR, suppG, suppB, normalizeParam1,paramGamma, resizeW, resizeH, ratio;
		int moveX, moveY, bgExtract;
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
            ("bg-extract,r", po::value<int>(&bgExtract)->default_value(1), 
                  "1: Effectuer l'extracion, 0:ne pas extraire le fond")
            ("sppress-R,r", po::value<float>(&suppR)->default_value(0.0f), 
                  "red suppression level [0.0;1.0]")
            ("sppress-G,g", po::value<float>(&suppG)->default_value(1.0f), 
                  "green suppression level [0.0;1.0]")
            ("sppress-B,b", po::value<float>(&suppB)->default_value(0.0f), 
                  "blue suppression level [0.0;1.0]")
            ("normalizeParam1,np1", po::value<float>(&normalizeParam1)->default_value(0.4f), 
                  "normalizeParam1 [0.0;1.0]?")
            ("gamma", po::value<float>(&paramGamma)->default_value(0.1f), 
                  "srcGamma [0.0;1.0]?")
            ("resizeW", po::value<float>(&resizeW)->default_value(0.f), 
                  "srcResize height (px)")
            ("resizeH", po::value<float>(&resizeH)->default_value(0.f), 
                  "srcResize width (px)")
            ("moveX", po::value<int>(&moveX)->default_value(0), 
                  "translate X (px)")
            ("moveY", po::value<int>(&moveY)->default_value(0), 
                  "translate X (px)")
            ("ratio", po::value<float>(&ratio)->default_value(0.f), 
                  "image ratio for crop")
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
            ("nothing", po::value< std::vector<std::string> >(), "nothing")
            ;

        po::options_description cmdline_options;
        cmdline_options.add(generic).add(config).add(hidden);

        po::options_description config_file_options;
        config_file_options.add(config).add(hidden);

        po::options_description visible("Allowed options");
        visible.add(generic).add(config);
        
        po::positional_options_description p;
        p.add("nothing", -1);
        
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

        if (vm.count("nothing"))
        {
            std::cout << "Unused argument(s): " 
                 << vm["nothing"].as< std::vector<std::string> >() << "\n";
        }

        std::cout << "suppress: R=" << suppR << ", G=" << suppG << ", B=" << suppB << "\n";  
		
		/********************************************************
							PLUGINS GRAPHE
		*********************************************************/
		
		using namespace tuttle::host;
		TUTTLE_COUT( "__________________________________________________0" );
		Core::instance().getPluginCache().addDirectoryToPath( "./plugins" );
		// Core::instance().getPluginCache().scanPluginFiles();
		Core::instance().preload();

		TUTTLE_COUT( Core::instance().getImageEffectPluginCache() );

		TUTTLE_COUT( "__________________________________________________1 Nodes Creation" );
		
		Graph g;
		Graph::Node& source     = g.createNode( "fr.tuttle.imagemagickreader" ); //
		Graph::Node& cadre      = g.createNode( "fr.tuttle.imagemagickreader" );
		Graph::Node& background = g.createNode( "fr.tuttle.imagemagickreader" );
		Graph::Node& sourceBd 	= g.createNode( "fr.tuttle.bitdepth" );
		Graph::Node& cadreBd 	= g.createNode( "fr.tuttle.bitdepth" );
		Graph::Node& backgroundBd=g.createNode( "fr.tuttle.bitdepth" );
		Graph::Node& colorSuppr = g.createNode( "fr.tuttle.duranduboi.colorsuppress" ); // suppression BG
		Graph::Node& srcInvertAlpha= g.createNode( "fr.tuttle.invert" );
		Graph::Node& cadreRGBA=g.createNode( "fr.tuttle.invert" );
		Graph::Node& bgRGBA=g.createNode( "fr.tuttle.invert" );
		Graph::Node& normalize 	= g.createNode( "fr.tuttle.duranduboi.normalize" ); // amelio qualité masque alpha
		Graph::Node& srcGamma  	= g.createNode( "fr.tuttle.gamma" ); // amelio qualité masque alpha
		Graph::Node& srcResize 	= g.createNode( "fr.tuttle.resize" );
		//Graph::Node& move2d 	= g.createNode( "fr.tuttle.move2d" );
		Graph::Node& mergeSRC_BG  	= g.createNode( "fr.tuttle.merge" );
		Graph::Node& mergeFRAME 	= g.createNode( "fr.tuttle.merge" );
		Graph::Node& write  	= g.createNode( "fr.tuttle.jpegwriter" );
		
		Graph::Node& crop  	= g.createNode( "fr.tuttle.crop" );
		
		//Graph::Node& writeKey	= g.createNode( "fr.tuttle.jpegwriter" );
		//Graph::Node& writeMerge1	= g.createNode( "fr.tuttle.jpegwriter" );
		//Graph::Node& writeKey    = g.createNode( "fr.tuttle.imagemagickwriter" ); // aucun effet, setParam doit pas fonctionner

		TUTTLE_COUT( "__________________________________________________2 Param nodes" );
		// Setup parameters
		source.getParam( "filename" ).set( src );
		cadre.getParam( "filename" ).set( frame );
		background.getParam( "filename" ).set( bg );
		source.getParam( "flip" ).set( true );
		cadre.getParam( "flip" ).set( true );
		background.getParam( "flip" ).set( true );
		// passe de 8 bits à 32 (floats)
		sourceBd.getParam( "outputBitDepth" ).set( "float" );
		cadreBd.getParam( "outputBitDepth" ).set( "float" );
		backgroundBd.getParam( "outputBitDepth" ).set( "float" );

		colorSuppr.getParam( "greenRate" ).set( suppG );
		colorSuppr.getParam( "blueRate" ).set( suppB );
		colorSuppr.getParam( "redRate" ).set( suppR );
		colorSuppr.getParam( "output" ).set( 2 ); // 2: modifie l'image et sort la couche alpha
		//TUTTLE_COUT_VAR(srcInvertAlpha);
		srcInvertAlpha.getParam( "processR" ).set( false );
		srcInvertAlpha.getParam( "processG" ).set( false );
		srcInvertAlpha.getParam( "processB" ).set( false );
		srcInvertAlpha.getParam( "processA" ).set( true );
		cadreRGBA.getParam( "processR" ).set( false );
		cadreRGBA.getParam( "processG" ).set( false );
		cadreRGBA.getParam( "processB" ).set( false );
		cadreRGBA.getParam( "processA" ).set( true );
		bgRGBA.getParam( "processR" ).set( false );
		bgRGBA.getParam( "processG" ).set( false );
		bgRGBA.getParam( "processB" ).set( false );
		bgRGBA.getParam( "processA" ).set( true );
		normalize.getParam( "mode" ).set( 1 );
		normalize.getParam( "srcColorMin" ).setAtIndex( normalizeParam1, 3 ); // 0.2,  3
		normalize.getParam( "processR" ).set( false );
		normalize.getParam( "processG" ).set( false );
		normalize.getParam( "processB" ).set( false );
		normalize.getParam( "processA" ).set( true );
		
		srcResize.getParam( "size" ).set( resizeW, resizeH );
		
		//crop.getParam( "presets" ).set("1.77 (16/9e) bands");//.set( 16./9. );
		//crop.getParam( "presets" ).set(2);
		
		srcGamma.getParam( "gammaType" ).set( "RGBA" );
		srcGamma.getParam( "alpha" ).set( paramGamma );
		mergeSRC_BG.getParam( "mergingFunction" ).set( "matte" );
		mergeSRC_BG.getParam( "rod" ).set( "B" ); 
		mergeSRC_BG.getParam( "offsetA" ).set( moveX, moveY );
		mergeFRAME.getParam( "mergingFunction" ).set( 19 );
		write.getParam( "premult" ).set( false );
		write.getParam( "filename" ).set( out );
		//writeKey.getParam( "premult" ).set( false );
		//writeKey.getParam( "filename" ).set( "data/key/outputKey.jpg" );
		//writeMerge1.getParam( "premult" ).set( false );
		//writeMerge1.getParam( "filename" ).set( "data/key/outputMerge1.jpg" );
		
		TUTTLE_COUT( "__________________________________________________3 Connect nodes" );
		// passage des trois images 8 bits => float
		g.connect( source, sourceBd );
		if(frame!="noframe"){
			g.connect( cadre, cadreBd );
			g.connect( cadreBd, cadreRGBA );
		}else{
		}
		g.connect( background, backgroundBd );
		if(ratio>0.1f){
			std::cout<< "crop with ratio:" <<ratio<<std::endl;
			g.connect( backgroundBd, crop ); // crop
			g.connect( crop, bgRGBA );
		}else{
			// no crop
			TUTTLE_COUT( "no crop" );
			g.connect( backgroundBd, bgRGBA );
		}
		
		// suppression de BG 
		if(bgExtract){
			g.connect( sourceBd, colorSuppr );
			g.connect( colorSuppr, srcInvertAlpha );
			// amélio Alpha
			g.connect( srcInvertAlpha, srcGamma );
		}
		else{
			//mergeSRC_BG.getParam( "mergingFunction" ).set( "copy" ); // only A
			g.connect( sourceBd, srcInvertAlpha );
		}

		std::cout<< "resizeW:" <<resizeW <<" resizeH:" <<resizeH <<" moveX:" <<moveX <<" moveY:" <<moveY <<std::endl;
		if(resizeW<1.f || resizeH < 1.f)
		{
			TUTTLE_COUT( "no resize" );
			if(bgExtract)
				g.connect( srcGamma, mergeSRC_BG.getClip("A") );
			else
				g.connect( srcInvertAlpha, mergeSRC_BG.getClip("A") );
		}
		else
		{
			TUTTLE_COUT( "resize" );
			if(bgExtract){
				g.connect( srcGamma, srcResize );
			}
			else{
				g.connect( srcInvertAlpha, srcResize );
			}
			g.connect( srcResize, mergeSRC_BG.getClip("A") );
		}
		g.connect( bgRGBA, mergeSRC_BG.getClip("B") );
		
		
		if(frame!="noframe"){
			g.connect( cadreRGBA, mergeFRAME.getClip("A") );
			g.connect( mergeSRC_BG, mergeFRAME.getClip("B") );
			g.connect( mergeFRAME, write );
		}else{
			g.connect( mergeSRC_BG, write );
		}
		
		//g.connect( srcInvertAlpha, writeKey );
		//g.connect( mergeSRC_BG, writeMerge1 ); // test 

		std::list<std::string> outputs;
		outputs.push_back( write.getName() );
		//outputs.push_back( writeKey.getName() );
		//outputs.push_back( writeMerge1.getName() );
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

