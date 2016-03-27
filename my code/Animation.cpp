// *******************************************************
// CS 174a Graphics Example Code
// Animation.cpp - The main file and program start point.  The class definition here describes how to display an Animation and how it will react to key and mouse input.  Right now it has 
// very little in it - you will fill it in with all your shape drawing calls and any extra key / mouse controls.  

// Now go down to display() to see where the sample shapes are drawn, and to see where to fill in your own code.

#include "../CS174a template/Utilities.h"
#include "../CS174a template/Shapes.h"
#include "../CS174a template/GLUT_Port.h"
int g_width = 800, g_height = 800 ;


// *******************************************************	
// When main() is called it creates an Animation object, which registers itself as a displayable object to our other class GL_Context -- which OpenGL is told to call upon every time a
// draw / keyboard / mouse event happens.
class Animation : public DisplayObject
{
	KeyMap<Animation> keys;
	GLUT_Port g;
	Matrix4d projection_transform, camera_transform;		
	Vector3d thrust;

	Cube* m_plane;
	Sphere* m_sphere;
	Tree* m_tree;
	Bee* m_bee;
	
	bool looking, animate, gouraud, color_normals, solid;
	float animation_time;

public:
	// projection_transform:  The matrix that determines how depth is treated.  It projects 3D points onto a plane.
	Animation() : projection_transform( Perspective( 60, float(g_width)/g_height, .1f, 100 ) ),	
		camera_transform( Matrix4d::Identity() ),		g( Vector2i(800, 20), Vector2i(600, 600), "CS174a Template Code"), looking(0), animation_time(0), animate(0), 
		gouraud(0), color_normals(0), solid(0), thrust( Vector3d::Zero() )
	{			
		g.registerDisplayObject(*this);
		g.init( "vshader.glsl", "fshader.glsl" );
			
		glClearColor( .0f, .0f, .0f, 1 );								// Background color

		// Plane
		double mat_plane[16] = {100, 0, 0, 0,
								0, 0.1, 0, 0,
								0, 0, 100, 0,
								0, 0, 0, 1};
		Matrix4d plane = Matrix4d(mat_plane);
		m_plane = new Cube(plane);

		// Tree

		m_tree = new Tree(Matrix4d::Identity(), animation_time);
		m_bee = new Bee(Matrix4d::Identity(), animation_time);
	}
	
	void update_camera( float animation_delta_time, Vector2d &window_steering )
	{
		const unsigned leeway = 70, border = 50;
		float degrees_per_frame = .02f * animation_delta_time;
		float meters_per_frame = 10.f * animation_delta_time; 
		cout << animation_time << endl;
																									// Determine camera rotation movement first
		Vector2f movement_plus  ( window_steering[0] + leeway, window_steering[1] + leeway );		// movement[] is mouse position relative to canvas center; leeway is a tolerance from the center.
		Vector2f movement_minus ( window_steering[0] - leeway, window_steering[1] - leeway );
		bool outside_border = false;
	
		for( int i = 0; i < 2; i++ )
			if ( abs( window_steering[i] ) > g.get_window_size()[i]/2 - border )	outside_border = true;		// Stop steering if we're on the outer edge of the canvas.

		for( int i = 0; looking && outside_border == false && i < 2; i++ )			// Steer according to "movement" vector, but don't start increasing until outside a leeway window from the center.
		{
			float angular_velocity = ( ( movement_minus[i] > 0) * movement_minus[i] + ( movement_plus[i] < 0 ) * movement_plus[i] ) * degrees_per_frame;	// Use movement's quantity conditionally
			camera_transform = Affine3d( AngleAxisd( angular_velocity, Vector3d( i, 1-i, 0 ) ) ) * camera_transform;			// On X step, rotate around Y axis, and vice versa.			
		}
		camera_transform = Affine3d(Translation3d( meters_per_frame * thrust )) * camera_transform;		// Now translation movement of camera, applied in local camera coordinate frame
	}
	
	// *******************************************************	
	// display(): called once per frame, whenever OpenGL decides it's time to redraw.
	virtual void display( float animation_delta_time, Vector2d &window_steering )
	{		
		if( animate ) animation_time += animation_delta_time;

		update_camera( animation_delta_time , window_steering );

		int basis_id = 0;

		float atime = animation_time * 10;
		Matrix4d model_transform = Matrix4d::Identity();
		Matrix4d std_model = model_transform;

		*m_tree = Tree(Matrix4d::Identity(), atime);
		// Start coding here!!!! 
		m_bee->timepassby(atime);

		// Plane
		model_transform = std_model * Affine3d(Translation3d(5, -5, -60)).matrix();				// Position
		glUniform4fv(g_addrs->color_loc, 1, Vector4f(.0f, .7f, .9f, 1).data());			// Color
		m_plane->draw ( projection_transform, camera_transform, model_transform, "" );

		// Tree
		model_transform = std_model * Affine3d(Translation3d(4, -5, -40)).matrix();				// Position
		glUniform4fv( g_addrs->color_loc, 1, Vector4f( .0f, .6f ,.2f ,1 ).data());			// Color
		m_tree->draw( basis_id++, projection_transform, camera_transform, model_transform, "");

		// Leg
		model_transform = std_model * Affine3d(Translation3d(4, 1+2*(abs(20.0 - ((int)atime % 41)))/10, -40)).matrix();				// Position
		model_transform *= Affine3d(AngleAxisd((-PI / 60 * atime), Vector3d(0, 1, 0))).matrix();
		model_transform *= Affine3d(Translation3d(20, 0, 0)).matrix();
		m_bee->draw(basis_id++, projection_transform, camera_transform, model_transform, "");
	}


	virtual void update_debug_strings( std::map< std::string, std::string >&	 debug_screen_strings )		// Strings this particular class contributes to the UI
	{ 
		debug_screen_strings["animation time:"] = "Animation time:  " + std::to_string( animation_time ); 
		debug_screen_strings["animate:"] = "Animation " + string( animate ? "on" : "off" ); 
		debug_screen_strings["zthrust"] = "Z thrust:  " + std::to_string( thrust[2] );
	}
	
	void forward() { thrust[2] =  1; }  void back()  { thrust[2] = -1; }    void up()    { thrust[1] = -1; }
	void left()    { thrust[0] =  1; }  void right() { thrust[0] = -1; }    void down()  { thrust[1] =  1; }
	void stopX()   { thrust[0] =  0; }  void stopY() { thrust[1] =  0; }    void stopZ() { thrust[2] =  0; }
	void toggleLooking() { looking = !looking; }
	void toggleColorNormals() { color_normals = !color_normals; glUniform1i( g_addrs->COLOR_NORMALS_loc, color_normals); }
	void toggleGouraud() { gouraud = !gouraud; glUniform1i( g_addrs->GOURAUD_loc, gouraud);}
	void toggleAnimate() { animate = !animate; }
	void reset() { camera_transform = Matrix4d::Identity(); }
	void roll_left() { camera_transform  *= Affine3d( AngleAxisd( 3 * PI /180, Vector3d( 0, 0,  1 ) ) ).matrix(); }
	void roll_right() { camera_transform *= Affine3d( AngleAxisd( 3 * PI /180, Vector3d( 0, 0, -1 ) ) ).matrix(); }
	void toggleSolid() { solid = !solid; glUniform1i( g_addrs->SOLID_loc, solid); glUniform4fv( g_addrs->SOLID_COLOR_loc, 1, Vector4f(Vector4f::Random()).data() );}
	
	// *******************************************************	
	// init_keys():  Define any extra keyboard shortcuts here
	virtual	void init_keys() 
	{
		keys.addHandler		( 'w', 0,					Callback<Animation>( &Animation::forward , this ) );
		keys.addHandler		( 'a', 0,					Callback<Animation>( &Animation::left    , this ) );
		keys.addHandler		( 's', 0,					Callback<Animation>( &Animation::back    , this ) );
		keys.addHandler		( 'd', 0,					Callback<Animation>( &Animation::right   , this ) );
		keys.addHandler		( ' ', 0,					Callback<Animation>( &Animation::up      , this ) );
		keys.addHandler		( 'z', 0,					Callback<Animation>( &Animation::down    , this ) );
		
		keys.addHandler		( 'w', GLUT_Port::RELEASE,  Callback<Animation>( &Animation::stopZ , this ) );
		keys.addHandler		( 'a', GLUT_Port::RELEASE,  Callback<Animation>( &Animation::stopX , this ) );
		keys.addHandler		( 's', GLUT_Port::RELEASE,  Callback<Animation>( &Animation::stopZ , this ) );
		keys.addHandler		( 'd', GLUT_Port::RELEASE,  Callback<Animation>( &Animation::stopX , this ) );
		keys.addHandler		( ' ', GLUT_Port::RELEASE,  Callback<Animation>( &Animation::stopY , this ) );
		keys.addHandler		( 'z', GLUT_Port::RELEASE,  Callback<Animation>( &Animation::stopY , this ) );
		
		keys.addHandler		( '.', 0,                   Callback<Animation>( &Animation::roll_left , this ) );
		keys.addHandler		( ',', 0,                   Callback<Animation>( &Animation::roll_right , this ) );
		keys.addHandler		( 'r', 0,                   Callback<Animation>( &Animation::reset , this ) );
		keys.addHandler		( 'f', 0,                   Callback<Animation>( &Animation::toggleLooking , this ) );
		
		keys.addHandler		( 'n', GLUT_Port::ALT,      Callback<Animation>( &Animation::toggleColorNormals , this ) );
		keys.addHandler		( 'g', GLUT_Port::ALT,      Callback<Animation>( &Animation::toggleGouraud , this ) );
		keys.addHandler		( 's', GLUT_Port::ALT,      Callback<Animation>( &Animation::toggleSolid , this ) );
		keys.addHandler		( 't', GLUT_Port::ALT,      Callback<Animation>( &Animation::toggleAnimate , this ) );

		update_controls< Animation >( keys );
	}
	// This must go in every class that has its own KeyMap and keyboard shortcuts
	virtual void handle_key( unsigned char key, unsigned char mods )	{	if( keys.hasHandler( key, mods ) )		keys.getHandler( key, mods )	();		}
	
};

int main() 
{
	Animation a;
	while(1)
	{
		glutMainLoopEvent();		// Run the OpenGL event loop forever
		GLUT_Port::idle();
	}
}