#ifndef OGRE_APPLICATION_H_
#define OGRE_APPLICATION_H_

#include <exception>
#include <string>

#include "OGRE/OgreRoot.h"
#include "OGRE/OgreRenderSystem.h"
#include "OGRE/OgreRenderWindow.h"
#include "OGRE/OgreWindowEventUtilities.h"
#include "OGRE/OgreManualObject.h"
#include "OGRE/OgreEntity.h"
#include "OIS/OIS.h"

namespace ogre_application {


	/* A useful type to define */
	typedef std::vector<Ogre::String> Strings;

	/* Our exception type */
	class OgreAppException: public std::exception
    {
		private:
		    std::string message_;
	    public:
			OgreAppException(std::string message) : message_(message) {};
			virtual const char* what() const throw() { return message_.c_str(); };
	};

	/* Our Ogre application */
	class OgreApplication :
	    public Ogre::FrameListener, // Derive from FrameListener to be able to have render event callbacks
        public Ogre::WindowEventListener // Derive from FrameListener to be able to have window event callbacks
		{

        public:
            OgreApplication(void);
            void Init(void); // Call Init() before running the main loop
			void CreateCube(void); // Create the geometry for a single cube
			void CreateMultipleCubes(void); // Create multiple entities of a cube
			void MainLoop(void); // Keep application active

			void CreateCylinder(void); // Create the geometry for a single Cylinder
			void CreateMultipleCylinders(void);
			void CreateTorus(Ogre::String object_name, Ogre::String material_name, float loop_radius = 0.6, float circle_radius = 0.2, int num_loop_samples = 90, int num_circle_samples = 30); // Create an object to show on the screen
			void CreateMultipleTorus(void);
			/* Transfromation demo */
			void TransformEntities(float timer);

        private:
			// Create root that allows us to access Ogre commands
            std::auto_ptr<Ogre::Root> ogre_root_;
            // Main Ogre window
            Ogre::RenderWindow* ogre_window_;

			/* Transfromation demo */
			bool animating_; // Whether animation is on or off
			bool space_down_; // Whether space key was pressed
			float timer_; // Timer for animation
			#define NUM_ELEMENTS 6 // Number of elements in the chain
			#define NUM_ELEMENTS_TORUS 2
			Ogre::SceneNode* torus_[NUM_ELEMENTS_TORUS]; 
			Ogre::SceneNode* cylinder_[NUM_ELEMENTS*2];
			// Input managers
			OIS::InputManager *input_manager_;
			OIS::Mouse *mouse_;
			OIS::Keyboard *keyboard_;

			/* Methods to initialize the application */
			void InitRootNode(void);
			void InitPlugins(void);
			void InitRenderSystem(void);
			void InitWindow(void);
			void InitViewport(void);
			void InitEvents(void);
			void InitOIS(void);
			void LoadMaterials(void);

			/* Methods to handle events */
			bool frameRenderingQueued(const Ogre::FrameEvent& fe);
			void windowResized(Ogre::RenderWindow* rw);

    }; // class OgreApplication

} // namespace ogre_application;

#endif // OGRE_APPLICATION_H_
