#include "ogre_application.h"
#include "bin/path_config.h"

namespace ogre_application {

/* Some configuration constants */
/* They are written here as global variables, but ideally they should be loaded from a configuration file */

/* Initialization */
const Ogre::String config_filename_g = "";
const Ogre::String plugins_filename_g = "";
const Ogre::String log_filename_g = "Ogre.log";

/* Main window settings */
const Ogre::String window_title_g = "Demo";
const Ogre::String custom_window_capacities_g = "";
const unsigned int window_width_g = 800;
const unsigned int window_height_g = 600;
const bool full_screen_g = false;

/* Viewport and camera settings */
float viewport_width_g = 0.95f;
float viewport_height_g = 0.95f;
float viewport_left_g = (1.0f - viewport_width_g) * 0.5f;
float viewport_top_g = (1.0f - viewport_height_g) * 0.5f;
unsigned short viewport_z_order_g = 100;
float camera_near_clip_distance_g = 0.1;
float camera_far_clip_distance_g = 1000.0;
float camera_field_of_view_g = 20.0f * Ogre::Math::PI / 180.0f;
Ogre::Vector3 camera_position_g(50.0, 0.0, 0);
Ogre::Vector3 camera_look_at_g(0.0, 0.0, 0.0);
Ogre::Vector3 camera_up_g(0.0, 1.0, 0.0);
const Ogre::ColourValue background_color_g(0.0, 0.0, 0.0);

/* Materials */
const Ogre::String material_directory_g = MATERIAL_DIRECTORY;


OgreApplication::OgreApplication(void){

    /* Don't do work in the constructor, leave it for the Init() function */
}


void OgreApplication::Init(void){

	/* Set default values for the variables */
	animating_ = true;
	space_down_ = false;
	timer_ = 0;

	input_manager_ = NULL;
	keyboard_ = NULL;
	mouse_ = NULL;
	
	/* Run all initialization steps */
    InitRootNode();
    InitPlugins();
    InitRenderSystem();
    InitWindow();
    InitViewport();
	InitEvents();
	InitOIS();
	LoadMaterials();
}


void OgreApplication::InitRootNode(void){

    try {
		
		/* We need to have an Ogre root to be able to access all Ogre functions */
        ogre_root_ = std::auto_ptr<Ogre::Root>(new Ogre::Root(config_filename_g, plugins_filename_g, log_filename_g));
		//ogre_root_->showConfigDialog();

    }
    catch (Ogre::Exception &e){
		throw(OgreAppException(std::string("Ogre::Exception: ") + std::string(e.what())));
    }
    catch(std::exception &e){
		throw(OgreAppException(std::string("std::Exception: ") + std::string(e.what())));
    }
}


void OgreApplication::InitPlugins(void){

    try {

		/* Load plugin responsible for OpenGL render system */
        Strings plugin_names;
        plugin_names.push_back("RenderSystem_GL");
		
        Strings::iterator iter = plugin_names.begin();
        Strings::iterator iter_end = plugin_names.end();
        for (; iter != iter_end; iter++){
            Ogre::String& plugin_name = (*iter);
            if (OGRE_DEBUG_MODE){
                plugin_name.append("_d");
            }
            ogre_root_->loadPlugin(plugin_name);
        }

    }
    catch (Ogre::Exception &e){
		throw(OgreAppException(std::string("Ogre::Exception: ") + std::string(e.what())));
    }
    catch(std::exception &e){
        throw(OgreAppException(std::string("std::Exception: ") + std::string(e.what())));
    }
}


void OgreApplication::InitRenderSystem(void){

    try {

        const Ogre::RenderSystemList& render_system_list = ogre_root_->getAvailableRenderers();
        if (render_system_list.size() == 0)
        {
			throw(OgreAppException(std::string("OgreApp::Exception: Sorry, no rendersystem was found.")));
        }

        Ogre::RenderSystem *render_system = render_system_list.at(0);
        ogre_root_->setRenderSystem(render_system);

    }
    catch (Ogre::Exception &e){
		throw(OgreAppException(std::string("Ogre::Exception: ") + std::string(e.what())));
    }
    catch(std::exception &e){
        throw(OgreAppException(std::string("std::Exception: ") + std::string(e.what())));
    }
}

        
void OgreApplication::InitWindow(void){

    try {

        /* Create main window for the application */
		bool create_window_automatically = false;
        ogre_root_->initialise(create_window_automatically, window_title_g, custom_window_capacities_g);

        Ogre::NameValuePairList params;
        params["FSAA"] = "0";
        params["vsync"] = "true";
        ogre_window_ = ogre_root_->createRenderWindow(window_title_g, window_width_g, window_height_g, full_screen_g, &params);

        ogre_window_->setActive(true);
        ogre_window_->setAutoUpdated(false);
    }
    catch (Ogre::Exception &e){
        throw(OgreAppException(std::string("Ogre::Exception: ") + std::string(e.what())));
    }
    catch(std::exception &e){
        throw(OgreAppException(std::string("std::Exception: ") + std::string(e.what())));
    }
}


void OgreApplication::InitViewport(void){

    try {

        /* Retrieve scene manager and root scene node */
        Ogre::SceneManager* scene_manager = ogre_root_->createSceneManager(Ogre::ST_GENERIC, "MySceneManager");
        Ogre::SceneNode* root_scene_node = scene_manager->getRootSceneNode();

        /* Create camera object */
        Ogre::Camera* camera = scene_manager->createCamera("MyCamera");
        Ogre::SceneNode* camera_scene_node = root_scene_node->createChildSceneNode("MyCameraNode");
        camera_scene_node->attachObject(camera);

        /* Create viewport */
        Ogre::Viewport *viewport = ogre_window_->addViewport(camera, viewport_z_order_g, viewport_left_g, viewport_top_g, viewport_width_g, viewport_height_g);

        viewport->setAutoUpdated(true);
        viewport->setBackgroundColour(background_color_g);

		/* Set camera */
        float ratio = float(viewport->getActualWidth()) / float(viewport->getActualHeight());
        camera->setAspectRatio(ratio);

		camera->setFOVy(Ogre::Radian(camera_field_of_view_g));

        camera->setNearClipDistance(camera_near_clip_distance_g);
        camera->setFarClipDistance(camera_far_clip_distance_g); 

		camera->setPosition(camera_position_g);
		camera->lookAt(camera_look_at_g);
		camera->setFixedYawAxis(true, camera_up_g);

		// Set up lighting
		/*scene_manager->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));*/

		// Create a Light and set its position
		/*Ogre::Light* light = scene_manager->createLight("MainLight");
		light->setPosition(1.0, 1.0, 1.0);*/
    }
    catch (Ogre::Exception &e){
        throw(OgreAppException(std::string("Ogre::Exception: ") + std::string(e.what())));
    }
    catch(std::exception &e){
        throw(OgreAppException(std::string("std::Exception: ") + std::string(e.what())));
    }
}


void OgreApplication::InitEvents(void){

	try {

		/* Add this object as a FrameListener for render events (see frameRenderingQueued event) */
		ogre_root_->addFrameListener(this);

		/* Add this object as a WindowEventListener to handle the window resize event */
		Ogre::WindowEventUtilities::addWindowEventListener(ogre_window_, this);

	}
    catch (Ogre::Exception &e){
		throw(OgreAppException(std::string("Ogre::Exception: ") + std::string(e.what())));
    }
    catch(std::exception &e){
		throw(OgreAppException(std::string("std::Exception: ") + std::string(e.what())));
    }
}


void OgreApplication::InitOIS(void){

	/* Initialize the Object Oriented Input System (OIS) */
	try {

		/* Initialize input manager */
		OIS::ParamList pl; // Parameter list passed to the input manager initialization
		size_t windowHnd = 0;
		std::ostringstream windowHndStr;
		ogre_window_->getCustomAttribute("WINDOW", &windowHnd);
		windowHndStr << windowHnd;
		pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
		pl.insert(std::make_pair(std::string("w32_mouse"), 
		std::string("DISCL_FOREGROUND" )));
		pl.insert(std::make_pair(std::string("w32_mouse"), 
		std::string("DISCL_NONEXCLUSIVE")));
		input_manager_ = OIS::InputManager::createInputSystem(pl);

		/*size_t hWnd = 0;
		ogre_window_->getCustomAttribute("WINDOW", &hWnd);
		input_manager_ = OIS::InputManager::createInputSystem(hWnd);*/

		/* Initialize keyboard and mouse */
		keyboard_ = static_cast<OIS::Keyboard*>(input_manager_->createInputObject(OIS::OISKeyboard, false));

		mouse_ = static_cast<OIS::Mouse*>(input_manager_->createInputObject(OIS::OISMouse, false));
		unsigned int width, height, depth;
		int top, left;
		ogre_window_->getMetrics(width, height, depth, left, top);
		const OIS::MouseState &ms = mouse_->getMouseState();
		ms.width = width;
		ms.height = height;

	}
    catch(std::exception &e){
        throw(OgreAppException(std::string("std::Exception: ") + std::string(e.what())));
    }
}


void OgreApplication::LoadMaterials(void){

    try {
		
		/* Load materials that can then be assigned to objects in the scene */
		Ogre::String resource_group_name = "MyGame";
		Ogre::ResourceGroupManager& resource_group_manager = Ogre::ResourceGroupManager::getSingleton();
		resource_group_manager.createResourceGroup(resource_group_name);
		bool is_recursive = false;
		resource_group_manager.addResourceLocation(material_directory_g, "FileSystem", resource_group_name, is_recursive);
		resource_group_manager.initialiseResourceGroup(resource_group_name);
		resource_group_manager.loadResourceGroup(resource_group_name);

	}
    catch (Ogre::Exception &e){
        throw(OgreAppException(std::string("Ogre::Exception: ") + std::string(e.what())));
    }
    catch(std::exception &e){
        throw(OgreAppException(std::string("std::Exception: ") + std::string(e.what())));
    }
}


void OgreApplication::CreateCube(void){

	try {
		/* Create a cube */

		/* Retrieve scene manager and root scene node */
		Ogre::SceneManager* scene_manager = ogre_root_->getSceneManager("MySceneManager");
		Ogre::SceneNode* root_scene_node = scene_manager->getRootSceneNode();

		/* Create the 3D object */
        Ogre::ManualObject* object = NULL;
        Ogre::String object_name = "Cube";
        object = scene_manager->createManualObject(object_name);
        object->setDynamic(false);

        /* Create triangle list for the object */
		Ogre::String material_name = "ObjectMaterial";
        object->begin(material_name, Ogre::RenderOperation::OT_TRIANGLE_LIST);

		/* Vertices of a cube */
		Ogre::Vector3 v0(-0.5, -0.5,  0.5);
		Ogre::Vector3 v1( 0.5, -0.5,  0.5);
		Ogre::Vector3 v2( 0.5,  0.5,  0.5);
		Ogre::Vector3 v3(-0.5,  0.5,  0.5);
		Ogre::Vector3 v4(-0.5, -0.5, -0.5);
		Ogre::Vector3 v5( 0.5, -0.5, -0.5);
		Ogre::Vector3 v6( 0.5,  0.5, -0.5);
		Ogre::Vector3 v7(-0.5,  0.5, -0.5);

		/* Normal of each face of the cube */
		Ogre::Vector3 n0( 0.0,  0.0,  1.0);
		Ogre::Vector3 n1( 1.0,  0.0,  0.0);
		Ogre::Vector3 n2( 0.0,  0.0, -1.0);
		Ogre::Vector3 n3(-1.0,  0.0,  0.0);
		Ogre::Vector3 n4( 0.0,  1.0,  0.0);
		Ogre::Vector3 n5( 0.0, -1.0,  0.0);

		/* Cube's color */
		Ogre::ColourValue clr0(0.0, 0.0, 1.0);
		Ogre::ColourValue clr1(1.0, 0.0, 1.0);
		Ogre::ColourValue clr2(1.0, 1.0, 1.0);
		Ogre::ColourValue clr3(0.0, 1.0, 0.0);
		Ogre::ColourValue clr4(0.0, 0.0, 1.0);
		Ogre::ColourValue clr5(1.0, 0.0, 0.0);
		Ogre::ColourValue clr6(1.0, 1.0, 0.0);
		Ogre::ColourValue clr7(0.0, 1.0, 0.0);
		
		/* This construction only partially uses shared vertices, so that we can assign appropriate vertex normals
		   to each face */
		/* Each face of the cube is defined by four vertices (with the same normal) and two triangles */
		object->position(v0);
		object->normal(n0);
		object->textureCoord(0, 0);
		object->colour(clr0);

		object->position(v1);
		object->normal(n0);
		object->textureCoord(1, 1);
		object->colour(clr1);

		object->position(v2);
		object->normal(n0);
		object->textureCoord(1, 1);
		object->colour(clr2);

		object->position(v3);
		object->normal(n0);
		object->textureCoord(0, 1);
		object->colour(clr3);
		
		object->position(v1);
		object->normal(n1);
		object->textureCoord(0, 0);
		object->colour(clr1);

		object->position(v5);
		object->normal(n1);
		object->textureCoord(1, 0);
		object->colour(clr5);

		object->position(v6);
		object->normal(n1);
		object->textureCoord(1, 1);
		object->colour(clr6);

		object->position(v2);
		object->normal(n1);
		object->textureCoord(0, 1);
		object->colour(clr2);

		object->position(v5);
		object->normal(n2);
		object->textureCoord(0, 0);
		object->colour(clr5);

		object->position(v4);
		object->normal(n2);
		object->textureCoord(1, 0);
		object->colour(clr4);
		
		object->position(v7);
		object->normal(n2);
		object->textureCoord(1, 1);
		object->colour(clr7);

		object->position(v6);
		object->normal(n2);
		object->textureCoord(0, 1);
		object->colour(clr6);

		object->position(v4);
		object->normal(n3);
		object->textureCoord(0, 0);
		object->colour(clr4);

		object->position(v0);
		object->normal(n3);
		object->textureCoord(1, 0);
		object->colour(clr0);

		object->position(v3);
		object->normal(n3);
		object->textureCoord(1, 1);
		object->colour(clr3);

		object->position(v7);
		object->normal(n3);
		object->textureCoord(0, 1);
		object->colour(clr7);

		object->position(v3);
		object->normal(n4);
		object->textureCoord(0, 0);
		object->colour(clr3);

		object->position(v2);
		object->normal(n4);
		object->textureCoord(1, 0);
		object->colour(clr2);

		object->position(v6);
		object->normal(n4);
		object->textureCoord(1, 1);
		object->colour(clr6);

		object->position(v7);
		object->normal(n4);
		object->textureCoord(0, 1);
		object->colour(clr7);

		object->position(v1);
		object->normal(n5);
		object->textureCoord(0, 0);
		object->colour(clr1);

		object->position(v0);
		object->normal(n5);
		object->textureCoord(1, 0);
		object->colour(clr0);

		object->position(v4);
		object->normal(n5);
		object->textureCoord(1, 1);
		object->colour(clr4);

		object->position(v5);
		object->normal(n5);
		object->textureCoord(0, 1);
		object->colour(clr5);

		for (int i = 0; i < 6; i++){
			object->triangle(i*4 + 0, i*4 + 1, i*4 + 3);
			object->triangle(i*4 + 1, i*4 + 2, i*4 + 3);
		}
   
		/* We finished the object */
        object->end();
		
        /* Convert triangle list to a mesh */
        Ogre::String mesh_name = "Cube";
        object->convertToMesh(mesh_name);

	}
    catch (Ogre::Exception &e){
        throw(OgreAppException(std::string("Ogre::Exception: ") + std::string(e.what())));
    }
    catch(std::exception &e){
        throw(OgreAppException(std::string("std::Exception: ") + std::string(e.what())));
    }
}

void OgreApplication::CreateCylinder(void){

	try {
		/* Create a Cylinder */

		/* Retrieve scene manager and root scene node */
		Ogre::SceneManager* scene_manager = ogre_root_->getSceneManager("MySceneManager");
		Ogre::SceneNode* root_scene_node = scene_manager->getRootSceneNode();

		   Ogre::Real cylinder_start=-0.5f;
		   Ogre::Real cylinder_length=1.0f;
		   Ogre::Real cylinder_raduis=1.0f;
		   const int cylinder_circle_resolution=120;
		   int loop_count;

		   Ogre::Degree theta =(Ogre::Degree)0;
		   Ogre::Degree alpha =(Ogre::Degree)360/cylinder_circle_resolution;

		   Ogre::Vector3 cylinder_circle1[cylinder_circle_resolution];
		   Ogre::Vector3 cylinder_circle2[cylinder_circle_resolution];

		   Ogre::Vector3  cylinder_circle1_center;
		   Ogre::Vector3  cylinder_circle2_center;

		   cylinder_circle1_center.x=-0.5;
		   cylinder_circle1_center.y=cylinder_start;
		   cylinder_circle1_center.z=0;

		   cylinder_circle2_center.x=-0.5;
		   cylinder_circle2_center.y=cylinder_start+cylinder_length;
		   cylinder_circle2_center.z=0;

		   for(int loop_count=0;loop_count<cylinder_circle_resolution;loop_count++)
		   {
			  theta=theta+alpha;
			  cylinder_circle1[loop_count].x=cylinder_start;
			  cylinder_circle1[loop_count].y=Ogre::Math::Sin(theta)*cylinder_raduis; 
			  cylinder_circle1[loop_count].z=Ogre::Math::Cos(theta)*cylinder_raduis; 

			  cylinder_circle2[loop_count].x=cylinder_start+cylinder_length;
			  cylinder_circle2[loop_count].y=Ogre::Math::Sin(theta)*cylinder_raduis; 
			  cylinder_circle2[loop_count].z=Ogre::Math::Cos(theta)*cylinder_raduis; 

		   }

		   	/* Create the 3D object */
			Ogre::ManualObject* object = NULL;
			Ogre::String object_name = "Cylinder";
			object = scene_manager->createManualObject(object_name);
			object->setDynamic(false);

		   //////////////cylinder left circular base of circle//////////////////////////
		   
			/* Create triangle list for the object */
		   Ogre::String material_name = "ObjectMaterial";
		   object->begin(material_name, Ogre::RenderOperation::OT_TRIANGLE_FAN);
		   object->colour(Ogre::ColourValue(0.0,0.0,1.0));
		   object->position(cylinder_circle1_center);
		   for( loop_count=0;loop_count<cylinder_circle_resolution;loop_count++)
		   {
			  object->position(cylinder_circle1[loop_count]);
		   }
		   object->position(cylinder_circle1[0]);
		   object->end();

		   //////////////cylinder curved surface//////////////////////////
		   object->begin(material_name, Ogre::RenderOperation::OT_TRIANGLE_LIST);
		   object->colour(Ogre::ColourValue(0.2,0.6,0.5));

		   for( loop_count=0;loop_count<cylinder_circle_resolution-1;loop_count++)
		   {  
			  object->position(cylinder_circle1[loop_count]);
			  object->position(cylinder_circle2[loop_count]);
			  object->position(cylinder_circle1[loop_count+1]);

			  object->position(cylinder_circle1[loop_count+1]);
			  object->position(cylinder_circle2[loop_count]);
			  object->position(cylinder_circle2[loop_count+1]);

		   }
		   object->position(cylinder_circle1[loop_count]);
		   object->position(cylinder_circle2[loop_count]);
		   object->position(cylinder_circle1[0]);

		   object->position(cylinder_circle1[0]);
		   object->position(cylinder_circle2[loop_count]);
		   object->position(cylinder_circle2[0]);

		   object->end();

		   //////////////cylinder right circular base of circle//////////////////////////
		   object->begin(material_name, Ogre::RenderOperation::OT_TRIANGLE_FAN);
		   object->colour(Ogre::ColourValue(0.0,0.0,1.0));
		   object->position(cylinder_circle2_center);
		   for( loop_count=cylinder_circle_resolution-1;loop_count>0;--loop_count)
		   {
			  object->position(cylinder_circle2[loop_count]);
		   }
		   object->position(cylinder_circle2[cylinder_circle_resolution-1]);
		   object->end();
   
		
        /* Convert triangle list to a mesh */
        Ogre::String mesh_name = "Cylinder";
        object->convertToMesh(mesh_name);

	}
    catch (Ogre::Exception &e){
        throw(OgreAppException(std::string("Ogre::Exception: ") + std::string(e.what())));
    }
    catch(std::exception &e){
        throw(OgreAppException(std::string("std::Exception: ") + std::string(e.what())));
    }
}

//void OgreApplication::CreateMultipleCubes(void){
//
//	try {
//		/* Create multiple entities of a cube */
//
//        /* Retrieve scene manager and root scene node */
//        Ogre::SceneManager* scene_manager = ogre_root_->getSceneManager("MySceneManager");
//        Ogre::SceneNode* root_scene_node = scene_manager->getRootSceneNode();
//
//        /* Create multiple entities of the cube mesh */
//		Ogre::String entity_name, prefix("Cube");
//		for (int i = 0; i < (NUM_ELEMENTS*2); i++){
//			/* Create entity */
//			entity_name = prefix + Ogre::StringConverter::toString(i);
//			Ogre::Entity *entity = scene_manager->createEntity(entity_name, "Cube");
//
//			/* Create a scene node for the entity */
//			/* The scene node keeps track of the entity's position */
//			cube_[i] = root_scene_node->createChildSceneNode(entity_name);
//			cube_[i]->attachObject(entity);
//		}
//
//    }
//    catch (Ogre::Exception &e){
//        throw(OgreAppException(std::string("Ogre::Exception: ") + std::string(e.what())));
//    }
//    catch(std::exception &e){
//        throw(OgreAppException(std::string("std::Exception: ") + std::string(e.what())));
//    }
//}

void OgreApplication::CreateTorus(Ogre::String object_name, Ogre::String material_name, float loop_radius, float circle_radius, int num_loop_samples, int num_circle_samples){

    try {
		/* Create a torus
		   The torus is built from a large loop with small circles around the loop */

        /* Retrieve scene manager and root scene node */
        Ogre::SceneManager* scene_manager = ogre_root_->getSceneManager("MySceneManager");
        Ogre::SceneNode* root_scene_node = scene_manager->getRootSceneNode();

        /* Create the 3D object */
        Ogre::ManualObject* object = NULL;
        object = scene_manager->createManualObject(object_name);
        object->setDynamic(false);

        /* Create triangle list for the object */
		object->begin(material_name, Ogre::RenderOperation::OT_TRIANGLE_LIST);

		/* Add vertices to the object */
		float theta, phi; // Angles for circles
		Ogre::Vector3 loop_center;
		Ogre::Vector3 vertex_position;
		Ogre::Vector3 vertex_normal;
		Ogre::ColourValue vertex_color;
				
		for (int i = 0; i < num_loop_samples; i++){ // large loop
			
			theta = Ogre::Math::TWO_PI*i/num_loop_samples; // loop sample (angle theta)
			loop_center = Ogre::Vector3(loop_radius*cos(theta), loop_radius*sin(theta), 0); // centre of a small circle

			for (int j = 0; j < num_circle_samples; j++){ // small circle
				
				phi = Ogre::Math::TWO_PI*j/num_circle_samples; // circle sample (angle phi)
				
				/* Define position, normal and color of vertex */
				vertex_normal = Ogre::Vector3(cos(theta)*cos(phi), sin(theta)*cos(phi), sin(phi));
				vertex_position = loop_center + vertex_normal*circle_radius;
				/*Ogre::Vector3(loop_center.x + local_normal.x*circle_radius, 
				                loop_center.y + local_normal.y*circle_radius, 
				     			loop_center.z + local_normal.z*circle_radius);*/
				vertex_color = Ogre::ColourValue(1.0 - ((float) i / (float) num_loop_samples), 
				                                 (float) i / (float) num_loop_samples, 
				                                 (float) j / (float) num_circle_samples);
				/*vertex_color = Ogre::ColourValue(0.0, 0.0, 1.0);*/

				/* Add them to the object */
				object->position(vertex_position);
				object->normal(vertex_normal);
				object->colour(vertex_color); 
			}
		}

		/* Add triangles to the object */
		for (int i = 0; i < num_loop_samples; i++){
			for (int j = 0; j < num_circle_samples; j++){
				// Two triangles per quad
				object->triangle(((i + 1) % num_loop_samples)*num_circle_samples + j, 
							     i*num_circle_samples + ((j + 1) % num_circle_samples),
								 i*num_circle_samples + j);
				object->triangle(((i + 1) % num_loop_samples)*num_circle_samples + j,
					             ((i + 1) % num_loop_samples)*num_circle_samples + ((j + 1) % num_circle_samples),
								 i*num_circle_samples + ((j + 1) % num_circle_samples));
			}
		}
		
		/* We finished the object */
        object->end();
		
        /* Convert triangle list to a mesh */
        object->convertToMesh(object_name);

    }
    catch (Ogre::Exception &e){
        throw(OgreAppException(std::string("Ogre::Exception: ") + std::string(e.what())));
    }
    catch(std::exception &e){
        throw(OgreAppException(std::string("std::Exception: ") + std::string(e.what())));
    }
}

void OgreApplication::CreateMultipleCylinders(void){

	try {
		/* Create multiple entities of a Cylinder */

        /* Retrieve scene manager and root scene node */
        Ogre::SceneManager* scene_manager = ogre_root_->getSceneManager("MySceneManager");
        Ogre::SceneNode* root_scene_node = scene_manager->getRootSceneNode();

		//create first cylinder which is called A as center
		Ogre::Entity *entity0 = scene_manager->createEntity("Cylinder0", "Cylinder");
		cylinder_[0] = root_scene_node->createChildSceneNode("Cylinder0",Ogre::Vector3( 0, 0, 0 ));
		cylinder_[0]->attachObject(entity0);
		cylinder_[0]->scale(4.0,0.25,0.25);
		//A left
		Ogre::Entity *entity1 = scene_manager->createEntity("Cylinder1", "Cylinder");
		cylinder_[1] = cylinder_[0]->createChildSceneNode("Cylinder1",Ogre::Vector3( -0.6, 0, 0 ));
		cylinder_[1]->attachObject(entity1);
		cylinder_[1]->scale(0.25,2,2);
		//A right
		Ogre::Entity *entity2 = scene_manager->createEntity("Cylinder2", "Cylinder");
		cylinder_[2] = cylinder_[0]->createChildSceneNode("Cylinder2",Ogre::Vector3( 0.6, 0, 0 ));
		cylinder_[2]->attachObject(entity2);
		cylinder_[2]->scale(0.25,2,2);
		//B left 1
		Ogre::Entity *entity3 = scene_manager->createEntity("Cylinder3", "Cylinder");
		cylinder_[3] = cylinder_[0]->createChildSceneNode("Cylinder3",Ogre::Vector3( -0.7, 0, 0 ));
		cylinder_[3]->attachObject(entity3);		
		cylinder_[3]->scale(0.5,0.2,0.2);
		cylinder_[3]->roll(Ogre::Degree( 90 ) );

		//B left 2
		Ogre::Entity *entity4 = scene_manager->createEntity("Cylinder4", "Cylinder");
		cylinder_[4] = cylinder_[0]->createChildSceneNode("Cylinder4",Ogre::Vector3( -0.7, 0, 0 ));
		cylinder_[4]->attachObject(entity4);
		cylinder_[4]->scale(0.5,0.2,0.2);
		cylinder_[4]->yaw( Ogre::Degree( 90 ) );

		//B right 1
		Ogre::Entity *entity5 = scene_manager->createEntity("Cylinder5", "Cylinder");
		cylinder_[5] = cylinder_[0]->createChildSceneNode("Cylinder5",Ogre::Vector3( 0.7, 0, 0 ));
		cylinder_[5]->attachObject(entity5);		
		cylinder_[5]->scale(0.5,0.2,0.2);
		cylinder_[5]->roll( Ogre::Degree( 90 ) );

		//B right 2
		Ogre::Entity *entity6 = scene_manager->createEntity("Cylinder6", "Cylinder");
		cylinder_[6] = cylinder_[0]->createChildSceneNode("Cylinder6",Ogre::Vector3( 0.7, 0, 0 ));
		cylinder_[6]->attachObject(entity6);
		cylinder_[6]->scale(0.5,0.2,0.2);
		cylinder_[6]->yaw( Ogre::Degree( 90 ) );

		//cube
		Ogre::Entity *entity7 = scene_manager->createEntity("Cylinder7", "Cube");
		cylinder_[7] = cylinder_[0]->createChildSceneNode("Cylinder7",Ogre::Vector3( 0.0, 0, 0 ));
		cylinder_[7]->attachObject(entity7);
		cylinder_[7]->scale(0.5,15,10);
		
		
		
    }
    catch (Ogre::Exception &e){
        throw(OgreAppException(std::string("Ogre::Exception: ") + std::string(e.what())));
    }
    catch(std::exception &e){
        throw(OgreAppException(std::string("std::Exception: ") + std::string(e.what())));
    }
}

void OgreApplication::CreateMultipleTorus(void){

	try {
		/* Create multiple entities of a Cylinder */

        /* Retrieve scene manager and root scene node */
        Ogre::SceneManager* scene_manager = ogre_root_->getSceneManager("MySceneManager");
        Ogre::SceneNode* root_scene_node = scene_manager->getRootSceneNode();

        /* Create multiple entities of the cube mesh */
		Ogre::String entity_name, prefix("Torus");
		for (int i = 0; i < NUM_ELEMENTS_TORUS; i++){
			/* Create entity */
			entity_name = prefix + Ogre::StringConverter::toString(i);
			Ogre::Entity *entity = scene_manager->createEntity(entity_name, "Torus");

			/* Create a scene node for the entity */
			/* The scene node keeps track of the entity's position */
			torus_[i] = cylinder_[0]->createChildSceneNode(entity_name);
			torus_[i]->scale(0.5,10,1);
			torus_[i]->attachObject(entity);
		}
		//setup torus
		torus_[0]->translate(0.7, 0, 0);
		torus_[0]->yaw(Ogre::Degree( 90 ));
		torus_[1]->translate(-0.7, 0, 0);
		torus_[1]->yaw(Ogre::Degree( 90 ));
    }
    catch (Ogre::Exception &e){
        throw(OgreAppException(std::string("Ogre::Exception: ") + std::string(e.what())));
    }
    catch(std::exception &e){
        throw(OgreAppException(std::string("std::Exception: ") + std::string(e.what())));
    }
}

void OgreApplication::MainLoop(void){

    try {

        /* Main loop to keep the application going */

        ogre_root_->clearEventTimes();

        while(!ogre_window_->isClosed()){
            ogre_window_->update(false);

            ogre_window_->swapBuffers();

            ogre_root_->renderOneFrame();

            Ogre::WindowEventUtilities::messagePump();
        }
    }
    catch (Ogre::Exception &e){
        throw(OgreAppException(std::string("Ogre::Exception: ") + std::string(e.what())));
    }
    catch(std::exception &e){
        throw(OgreAppException(std::string("std::Exception: ") + std::string(e.what())));
    }
}


bool OgreApplication::frameRenderingQueued(const Ogre::FrameEvent& fe){
  
	/* This event is called after a frame is queued for rendering */
	/* Do stuff in this event since the GPU is rendering and the CPU is idle */

	/* Keep animating if flag is on */
	if (animating_){
		/* Animate transformation */
		TransformEntities(timer_);
		timer_++;
	}

	/* Capture input */
	keyboard_->capture();
	mouse_->capture();

	/* Handle specific key events */
	if (keyboard_->isKeyDown(OIS::KC_SPACE)){
		space_down_ = true;
	}
	if ((!keyboard_->isKeyDown(OIS::KC_SPACE)) && space_down_){
		animating_ = !animating_;
		space_down_ = false;
	}
	if (keyboard_->isKeyDown(OIS::KC_ESCAPE)){
		timer_ = 0;
	}
 
    return true;
}


void OgreApplication::windowResized(Ogre::RenderWindow* rw){

	/* Update the window and aspect ratio when the window is resized */
	int width = rw->getWidth(); 
    int height = rw->getHeight();
      
    Ogre::SceneManager* scene_manager = ogre_root_->getSceneManager("MySceneManager");
    Ogre::Camera* camera = scene_manager->getCamera("MyCamera");

	if (camera != NULL){
		//std::cout << "1 " << (double)width/height << std::endl;
		camera->setAspectRatio((double)width/height);
    }

	const OIS::MouseState &ms = mouse_->getMouseState();
    ms.width = width;
    ms.height = height;

	ogre_window_->resize(width, height);
	ogre_window_->windowMovedOrResized();
	ogre_window_->update();
}

//not use in this program
// Create a rotation matrix based on an angle and an axis
Ogre::Matrix4 RotationMatrix(Ogre::Vector3 axis, Ogre::Radian angle){

	Ogre::Matrix3 mat;
	mat = Ogre::Matrix3::IDENTITY;
	mat.FromAngleAxis(axis, angle);
	return Ogre::Matrix4(mat);
}

//not use in this program
// Create a translation matrix based on a vector of translations (x, y, z)
Ogre::Matrix4 TranslationMatrix(Ogre::Vector3 trans){

	Ogre::Matrix4 mat;
	mat = Ogre::Matrix4::IDENTITY;
	mat.setTrans(trans);
	return mat;
}

//not use in this program
// Create a scaling matrix based on a vector of scale factors (x, y, z)
Ogre::Matrix4 ScalingMatrix(Ogre::Vector3 scale){

	Ogre::Matrix4 mat;
	mat = Ogre::Matrix4::IDENTITY;
	mat.setScale(scale);
	return mat;
}

//not use in this program
// Assign a transformation matrix to a scene node
void AssignTransf(Ogre::SceneNode* node, Ogre::Matrix4 transf){
	
	/* In many graphic frameworks, we would simply multiply our geometry by the transformation matrix.
	   However, OGRE stores the transformations of a node in a more efficient manner.
	   So, we need to decompose the transformation first into three components and then assign them
	   to the scene node.*/
	Ogre::Vector3 trans, scale;
	Ogre::Quaternion quat;
	
	transf.decomposition(trans, scale, quat);
//	node->setScale(scale);
//	node->setOrientation(quat);
//	node->setPosition(trans);
}


// Main function to animate chain of cubes
void OgreApplication::TransformEntities(float timer){

	//make center A roate around X axis
	cylinder_[0]->rotate(Ogre::Vector3(0,1,1), Ogre::Radian(0.01));
	//let sway slowly
	cylinder_[0]->translate(0.0, 0.0, 0.005);
	//rotate B with high speed
	cylinder_[3]->rotate(Ogre::Vector3(0,1,0), Ogre::Radian(0.1));
	cylinder_[4]->rotate(Ogre::Vector3(0,0,-1), Ogre::Radian(0.1));
	cylinder_[5]->rotate(Ogre::Vector3(0,1,0), Ogre::Radian(0.1));
	cylinder_[6]->rotate(Ogre::Vector3(0,0,-1), Ogre::Radian(0.1));
	//rotate C with low speed
	torus_[0]->rotate(Ogre::Vector3(0,0,-1), Ogre::Radian(0.05));
	torus_[1]->rotate(Ogre::Vector3(0,0,-1), Ogre::Radian(0.05));

}


} // namespace ogre_application;