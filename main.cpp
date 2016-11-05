#include <irrlicht.h>

#ifdef _IRR_ANDROID_PLATFORM_

#include <android_native_app_glue.h>
#include "android_tools.h"
#include "android/window.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;


enum GUI_IDS
{
	GUI_INFO_FPS,
	GUI_BUTTON_FLIP,
	GUI_BUTTON_BACKFLIP,
	GUI_BUTTON_ATTACK,
	GUI_BUTTON_JUMP,
};


class MyEventReceiver : public IEventReceiver
{
private:
	IrrlichtDevice* Device;
	android_app* AndroidApp;
	gui::IGUIElement * SpriteToMove;
	core::rect<s32> SpriteStartRect;
	core::position2d<irr::s32> TouchStartPos;
	s32 TouchID;
	bool isFlipping;
	bool isAttacking;
	bool isBackFlipping;
	bool isJumping;
	bool locked;
public:
	MyEventReceiver(android_app* app ) 
	: Device(0), AndroidApp(app), SpriteToMove(0), TouchID(-1),
		isFlipping(false), isBackFlipping(false), isAttacking(false), isJumping(false), locked(false)
	{
	}
	
	void Init(IrrlichtDevice *device)
	{
		Device = device;
	}

	virtual bool OnEvent(const SEvent& event)
	{
		if (event.EventType == EET_TOUCH_INPUT_EVENT)
		{
			SEvent fakeMouseEvent;
			fakeMouseEvent.EventType = EET_MOUSE_INPUT_EVENT;
			fakeMouseEvent.MouseInput.X = event.TouchInput.X;
			fakeMouseEvent.MouseInput.Y = event.TouchInput.Y;
			fakeMouseEvent.MouseInput.Shift = false;
			fakeMouseEvent.MouseInput.Control = false;
			fakeMouseEvent.MouseInput.ButtonStates = 0;
			fakeMouseEvent.MouseInput.Event = EMIE_COUNT;
			
			switch (event.TouchInput.Event)
			{
				case ETIE_PRESSED_DOWN:
				{
					if ( TouchID == -1 && !locked)
					{
						fakeMouseEvent.MouseInput.Event = EMIE_LMOUSE_PRESSED_DOWN;
				
						if (Device)
						{
							position2d<s32> touchPoint(event.TouchInput.X, event.TouchInput.Y);
							IGUIElement * flip_button = Device->getGUIEnvironment()->getRootGUIElement()->getElementFromId ( GUI_BUTTON_FLIP );
							IGUIElement * backflip_button = Device->getGUIEnvironment()->getRootGUIElement()->getElementFromId ( GUI_BUTTON_BACKFLIP );
							IGUIElement * attack_button = Device->getGUIEnvironment()->getRootGUIElement()->getElementFromId ( GUI_BUTTON_ATTACK );
							IGUIElement * jump_button = Device->getGUIEnvironment()->getRootGUIElement()->getElementFromId ( GUI_BUTTON_JUMP );
							isFlipping = flip_button && flip_button->isPointInside(touchPoint);
							isBackFlipping = backflip_button && backflip_button->isPointInside(touchPoint);
							isAttacking = attack_button && attack_button->isPointInside(touchPoint);
							isJumping = jump_button && jump_button->isPointInside(touchPoint);
							locked = isFlipping || isBackFlipping || isAttacking || isJumping;
						}
					}
					break;
				}
				case ETIE_MOVED:
					if ( TouchID == event.TouchInput.ID )
					{
						fakeMouseEvent.MouseInput.Event = EMIE_MOUSE_MOVED;
						fakeMouseEvent.MouseInput.ButtonStates = EMBSM_LEFT;
					}
					break;
				case ETIE_LEFT_UP:
					if ( TouchID == event.TouchInput.ID )
					{
						fakeMouseEvent.MouseInput.Event = EMIE_LMOUSE_LEFT_UP;
					}
					break;
				default:
					break;
			}
			
			if ( fakeMouseEvent.MouseInput.Event != EMIE_COUNT && Device )
			{
				Device->postEventFromUser(fakeMouseEvent);
			}
		}
		else if ( event.EventType == EET_GUI_EVENT )
		{
			switch(event.GUIEvent.EventType)
			{
				case EGET_EDITBOX_ENTER:
					if ( event.GUIEvent.Caller->getType() == EGUIET_EDIT_BOX )
					{
						if( Device->getGUIEnvironment() )
							Device->getGUIEnvironment()->setFocus(NULL);
						android::setSoftInputVisibility(AndroidApp, false);
					}
				break;					
                case EGET_ELEMENT_FOCUS_LOST:
					if ( event.GUIEvent.Caller->getType() == EGUIET_EDIT_BOX )
					{
						android::setSoftInputVisibility(AndroidApp, false);
					}
                break;					
                case EGET_ELEMENT_FOCUSED:					
					if ( event.GUIEvent.Caller->getType() == EGUIET_EDIT_BOX )
					{
						android::setSoftInputVisibility(AndroidApp, true);
					}
                break;
				default:
					break;
			}
		}
		
		return false;
	}
	
	bool IsFlipping() const
	{
		return isFlipping;
	}
	void FlipComplete()
	{
		isFlipping = locked = false;
	}	
	
	bool IsAttacking() const
	{
		return isAttacking;
	}
	void AttackComplete()
	{
		isAttacking = locked = false;
	}	
	
	bool IsBackFlipping() const
	{
		return isBackFlipping;
	}
	void BackFlippingComplete()
	{
		isBackFlipping = locked = false;
	}
    bool IsJumping() const
	{
		return isJumping;
	}
	void JumpingComplete()
	{
		isJumping = locked = false;
	}
};

void mainloop( IrrlichtDevice *device, IGUIStaticText * infoText, IAnimatedMeshSceneNode* lb, MyEventReceiver& receiver)
{
	u32 loop = 0;
	static u32 runCounter = 0;
	bool set = true;
	lb->setTransitionTime(0.2);
	lb->setFrameLoop(10, 500);
	while(device->run())
	{
		if (device->isWindowActive())
		{
			if ( infoText )
			{
				stringw str = L"FPS:";
				str += (s32)device->getVideoDriver()->getFPS();
				infoText->setText ( str.c_str() );
			}
			
			if(receiver.IsAttacking()) {
				if (set) {
					lb->setFrameLoop(3550, 3800);
					lb->setTransitionTime(1);
					set=false;
				}
				else if(lb->getFrameNr() >= 3700) {
					receiver.AttackComplete();
					lb->setTransitionTime(0.2);
					lb->setFrameLoop(10, 500);
					set = true;
				}
			}

			if(receiver.IsBackFlipping()) {
				if (set) {
					lb->setFrameLoop(3820, 4050);
					lb->setTransitionTime(1);
					set=false;
				}
				else if(lb->getFrameNr() >= 3950) {
					receiver.BackFlippingComplete();
					lb->setTransitionTime(0.2);
					lb->setFrameLoop(10, 500);
					set = true;
				}
			}

			if(receiver.IsJumping()) {
				if (set) {
					lb->setFrameLoop(2170, 2400);
					lb->setTransitionTime(1);
					set=false;
				}
				else if(lb->getFrameNr() >= 2300) {
					receiver.JumpingComplete();
					lb->setTransitionTime(0.2);
					lb->setFrameLoop(10, 500);
					set = true;
				}
			}

			if(receiver.IsFlipping()) {
				if(set) {
					lb->setFrameLoop(2480, 2610);
					lb->setTransitionTime(1);
					set = false;
				} 
				else if(lb->getFrameNr() >= 2600){
					receiver.FlipComplete();
					lb->setTransitionTime(0.2);
					lb->setFrameLoop(10, 500);
					set = true;
				}
			}

			
			lb->animateJoints(true);		
			
			device->getVideoDriver()->beginScene(true, true, SColor(0,50,50,50));
			device->getSceneManager()->drawAll();
			device->getGUIEnvironment()->drawAll();
			device->getVideoDriver()->endScene ();
		}
		device->yield();
		++runCounter;
		++loop;
	}
}

void android_main(android_app* app)
{
	app_dummy();

	MyEventReceiver receiver(app);

	SIrrlichtCreationParameters param;

	param.DriverType = EDT_OGLES2;
	param.WindowSize = dimension2d<u32>(0,0);
	param.PrivateData = app;
	param.Bits = 24;
	param.ZBufferBits = 16;
	param.AntiAlias  = 0;
	param. EventReceiver = &receiver;

	IrrlichtDevice *device = createDeviceEx(param);
	if (device == 0) {
       	return;
	}
	
	IVideoDriver* driver = device->getVideoDriver();
	ISceneManager* smgr = device->getSceneManager();
	IGUIEnvironment* guienv = device->getGUIEnvironment();
	ILogger* logger = device->getLogger();
	IFileSystem * fs = device->getFileSystem();
	
	video::IImage *colorTexture = device->getVideoDriver()->createImage(video::ECF_A8R8G8B8, core::dimension2d<u32>(2, 2));
	colorTexture->fill(video::SColor(130, 0, 0, 255));
	device->getVideoDriver()->addTexture("blue", colorTexture);

	ANativeWindow* nativeWindow = static_cast<ANativeWindow*>(driver->getExposedVideoData().OGLESAndroid.Window);
	int32_t windowWidth = ANativeWindow_getWidth(app->window);
	int32_t windowHeight = ANativeWindow_getHeight(app->window);
	
	irr::android::SDisplayMetrics displayMetrics;
	memset(&displayMetrics, 0, sizeof displayMetrics);
	irr::android::getDisplayMetrics(app, displayMetrics);

	char strDisplay[1000];
	sprintf(strDisplay, "Window size:(%d/%d)\nDisplay size:(%d/%d)", windowWidth, windowHeight, displayMetrics.widthPixels, displayMetrics.heightPixels);
	logger->log(strDisplay);
	
	core::dimension2d<s32> dim(driver->getScreenSize());
	sprintf(strDisplay, "getScreenSize:(%d/%d)", dim.Width, dim.Height);
	logger->log(strDisplay);

   	stringc mediaPath = "media/";

	for ( u32 i=0; i < fs->getFileArchiveCount(); ++i )
	{
		IFileArchive* archive = fs->getFileArchive(i);
		if ( archive->getType() == EFAT_ANDROID_ASSET )
		{
			archive->addDirectoryToFileList(mediaPath);
			break;
		}
	}

	IGUISkin* skin = guienv->getSkin();
	IGUIFont* font = guienv->getFont(mediaPath + "bigfont.png");
	if (font) {
		skin->setFont(font);
	}

	IGUIStaticText *text = guienv->addStaticText(stringw(displayMetrics.xdpi).c_str(),
		rect<s32>(5,5,635,35), false, false, 0, GUI_INFO_FPS );
		
	// weaksauce
	s32 minLogoWidth = windowWidth/5;
	s32 yPos = windowHeight - 300;

	IGUIImage * backflip_button = guienv->addImage(driver->getTexture(mediaPath + "backflip.png"),
					core::position2d<s32>(100,yPos), true, 0, GUI_BUTTON_BACKFLIP);
	if ( backflip_button && backflip_button->getRelativePosition().getWidth() < minLogoWidth )
	{
		backflip_button->setScaleImage(false);
		core::rect<s32> logoPos(backflip_button->getRelativePosition());
		f32 scale = (f32)minLogoWidth/(f32)logoPos.getWidth();
		logoPos.LowerRightCorner.X = logoPos.UpperLeftCorner.X + minLogoWidth;
		logoPos.LowerRightCorner.Y = logoPos.UpperLeftCorner.Y + (s32)((f32)logoPos.getHeight()*scale);
		backflip_button->setRelativePosition(logoPos);
	}

	IGUIImage * attack_button = guienv->addImage(driver->getTexture(mediaPath + "attack.png"),
					core::position2d<s32>(minLogoWidth + 100,yPos), true, 0, GUI_BUTTON_ATTACK);
	if ( attack_button && attack_button->getRelativePosition().getWidth() < minLogoWidth )
	{
		attack_button->setScaleImage(true);
		core::rect<s32> logoPos(attack_button->getRelativePosition());
		f32 scale = (f32)minLogoWidth/(f32)logoPos.getWidth();
		logoPos.LowerRightCorner.X = logoPos.UpperLeftCorner.X + minLogoWidth;
		logoPos.LowerRightCorner.Y = logoPos.UpperLeftCorner.Y + (s32)((f32)logoPos.getHeight()*scale);
		attack_button->setRelativePosition(logoPos);
	}

	IGUIImage * jump_button = guienv->addImage(driver->getTexture(mediaPath + "jump.png"),
					core::position2d<s32>(minLogoWidth*2 + 100,yPos), true, 0, GUI_BUTTON_JUMP);
	if ( jump_button && jump_button->getRelativePosition().getWidth() < minLogoWidth )
	{
		jump_button->setScaleImage(true);
		core::rect<s32> logoPos(jump_button->getRelativePosition());
		f32 scale = (f32)minLogoWidth/(f32)logoPos.getWidth();
		logoPos.LowerRightCorner.X = logoPos.UpperLeftCorner.X + minLogoWidth;
		logoPos.LowerRightCorner.Y = logoPos.UpperLeftCorner.Y + (s32)((f32)logoPos.getHeight()*scale);
		jump_button->setRelativePosition(logoPos);
	}

	IGUIImage * flip_button = guienv->addImage(driver->getTexture(mediaPath + "flip.png"),
					core::position2d<s32>(minLogoWidth*3 + 100,yPos), true, 0, GUI_BUTTON_FLIP);
	if ( flip_button && flip_button->getRelativePosition().getWidth() < minLogoWidth )
	{
		flip_button->setScaleImage(true);
		core::rect<s32> logoPos(flip_button->getRelativePosition());
		f32 scale = (f32)minLogoWidth/(f32)logoPos.getWidth();
		logoPos.LowerRightCorner.X = logoPos.UpperLeftCorner.X + minLogoWidth;
		logoPos.LowerRightCorner.Y = logoPos.UpperLeftCorner.Y + (s32)((f32)logoPos.getHeight()*scale);
		flip_button->setRelativePosition(logoPos);
	}


	IAnimatedMesh* mesh = smgr->getMesh(mediaPath + "lb.x");
	if (!mesh)
	{
		device->closeDevice();
		device->drop();
       	return;
	}
	IAnimatedMeshSceneNode* lb = smgr->addAnimatedMeshSceneNode( mesh );
	if (lb)
	{
		//lb->setMaterialFlag(EMF_LIGHTING, false);
		lb->setFrameLoop(10, 500);
		lb->setAnimationSpeed(300);
		lb->setDebugDataVisible((scene::E_DEBUG_SCENE_TYPE)(lb->isDebugDataVisible()^scene::EDS_SKELETON));
		//lb->setDebugDataVisible(true);
		lb->setPosition(vector3df(0,10,25));
		lb->setScale(vector3df(2.0,2.0,2.0));
	}

	//ILightSceneNode* light = smgr->addLightSceneNode( 0, vector3df(200,300,-200), SColor(255,255,200,50), 500000.0f,-1);


 	//ISceneNode* floor = smgr->addCubeSceneNode(10.f, 0, -1, vector3df(0,-50,0), vector3df(0,0,0), vector3df(200,0.01,200));
	IAnimatedMesh *terrain_model = smgr->addHillPlaneMesh("groundPlane", // Name of the scenenode
	                       core::dimension2d<f32>(10.0f, 10.0f), // Tile size
	                       core::dimension2d<u32>(40, 40), // Tile count
	                       0, // Material
	                       0.0f, // Hill height
	                       core::dimension2d<f32>(0.0f, 0.0f), // countHills
	                       core::dimension2d<f32>(4.0f, 4.0f)); // textureRepeatCount
	                                      
	ISceneNode* terrain_node = smgr->addAnimatedMeshSceneNode(terrain_model);
	terrain_node->setMaterialTexture(0, driver->getTexture(mediaPath + "blueprint.png")); 
	terrain_node->setMaterialFlag(EMF_LIGHTING, false);
	terrain_node->setPosition(vector3df(0,-50,0));

	
	receiver.Init(device);

	smgr->addCameraSceneNode(0, vector3df(15,70,-120), vector3df(0,30,0));

	mainloop(device, text, lb, receiver);

	device->setEventReceiver(0);
	device->closeDevice();
	device->drop();
}

#endif	// defined(_IRR_ANDROID_PLATFORM_)

/*
**/
