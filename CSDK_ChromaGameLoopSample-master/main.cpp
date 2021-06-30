
//https://github.com/razerofficial/CSDK_ChromaGameLoopSample/blob/master/main.cpp

// CSDK_ChromaGameLoopSample.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

// When true, the sample will set Chroma effects directly from Arrays
// When false, the sample will use dynamic animations that set Chroma effects
// using the first frame of the dynamic animation.
#define USE_ARRAY_EFFECTS false

#include "Razer\ChromaAnimationAPI.h"
#include "HandleInput.h"
#include <array>
#include <chrono>
#include <conio.h>
#include <iostream>
#include <string>
#include <tchar.h>
#include <time.h>
#include <thread>

using namespace ChromaSDK;
using namespace std;

const float MATH_PI = 3.14159f;

#if !USE_ARRAY_EFFECTS

// This final animation will have a single frame
// Any color changes will immediately display in the next frame update.

const char* ANIMATION_FINAL_KEYBOARD = "Dynamic\\Final_Keyboard.chroma";


#endif

static bool _sWaitForExit = true;
static bool _sHotkeys = true;
static bool _sAmmo = true;
static int _sAmbientColor = 0;
static int _sIndexLandscape = -1;
static int _sIndexFire = -1;
static int _sIndexRainbow = -1;
static int _sIndexSpiral = -1;

// Function prototypes
void Cleanup();
void GameLoop();
int GetKeyColorIndex(int row, int column);
void InputHandler();
void Init();
int main();
void SetKeyColor(int* colors, int rzkey, int color);
void SetKeyColorRGB(int* colors, int rzkey, int red, int green, int blue);

static FChromaSDKScene _sScene;

void Init()
{
	if (ChromaAnimationAPI::InitAPI() != 0)
	{
		cerr << "Failed to load Chroma library!" << endl;
		exit(1);
	}

	ChromaSDK::APPINFOTYPE appInfo = {};

	_tcscpy_s(appInfo.Title, 256, _T("Razer Chroma CSDK Game Loop Sample Application"));
	_tcscpy_s(appInfo.Description, 1024, _T("A sample application using Razer Chroma SDK"));
	_tcscpy_s(appInfo.Author.Name, 256, _T("Razer"));
	_tcscpy_s(appInfo.Author.Contact, 256, _T("https://developer.razer.com/chroma"));

	//appInfo.SupportedDevice = 
	//    0x01 | // Keyboards
	//    0x02 | // Mice
	//    0x04 | // Headset
	//    0x08 | // Mousepads
	//    0x10 | // Keypads
	//    0x20   // ChromaLink devices
	appInfo.SupportedDevice = (0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20);
	//    0x01 | // Utility. (To specifiy this is an utility application)
	//    0x02   // Game. (To specifiy this is a game);
	appInfo.Category = 1;

	RZRESULT result = ChromaAnimationAPI::InitSDK(&appInfo);
	if (result != RZRESULT_SUCCESS)
	{
		cerr << "Failed to initialize Chroma!" << endl;
		exit(1);
	}
	Sleep(100); //wait for init
}

int GetKeyColorIndex(int row, int column)
{
	return Keyboard::MAX_COLUMN * row + column;
}

void SetKeyColor(int* colors, int rzkey, int color)
{
	int row = HIBYTE(rzkey);
	int column = LOBYTE(rzkey);
	colors[GetKeyColorIndex(row, column)] = color;
}

void SetKeyColorRGB(int* colors, int rzkey, int red, int green, int blue)
{
	SetKeyColor(colors, rzkey, ChromaAnimationAPI::GetRGB(red, green, blue));
}

const int GetColorArraySize1D(EChromaSDKDevice1DEnum device)
{
	const int maxLeds = ChromaAnimationAPI::GetMaxLeds((int)device);
	return maxLeds;
}

const int GetColorArraySize2D(EChromaSDKDevice2DEnum device)
{
	const int maxRow = ChromaAnimationAPI::GetMaxRow((int)device);
	const int maxColumn = ChromaAnimationAPI::GetMaxColumn((int)device);
	return maxRow * maxColumn;
}

#if !USE_ARRAY_EFFECTS

void SetupAnimation1D(const char* path, EChromaSDKDevice1DEnum device)
{
	int animationId = ChromaAnimationAPI::GetAnimation(path);
	if (animationId == -1)
	{
		animationId = ChromaAnimationAPI::CreateAnimationInMemory((int)EChromaSDKDeviceTypeEnum::DE_1D, (int)device);
		ChromaAnimationAPI::CopyAnimation(animationId, path);
		ChromaAnimationAPI::CloseAnimation(animationId);
		ChromaAnimationAPI::MakeBlankFramesName(path, 1, 0.1f, 0);
	}
}

void SetupAnimation2D(const char* path, EChromaSDKDevice2DEnum device)
{
	int animationId = ChromaAnimationAPI::GetAnimation(path);
	if (animationId == -1)
	{
		animationId = ChromaAnimationAPI::CreateAnimationInMemory((int)EChromaSDKDeviceTypeEnum::DE_2D, (int)device);
		ChromaAnimationAPI::CopyAnimation(animationId, path);
		ChromaAnimationAPI::CloseAnimation(animationId);
		ChromaAnimationAPI::MakeBlankFramesName(path, 1, 0.1f, 0);
	}
}
#endif

void SetStaticColor(int* colors, int color, int size)
{
	for (int i = 0; i < size; ++i)
	{
		colors[i] = color;
	}
}

int MultiplyColor(int color1, int color2) {
	int redColor1 = color1 & 0xFF;
	int greenColor1 = (color1 >> 8) & 0xFF;
	int blueColor1 = (color1 >> 16) & 0xFF;

	int redColor2 = color2 & 0xFF;
	int greenColor2 = (color2 >> 8) & 0xFF;
	int blueColor2 = (color2 >> 16) & 0xFF;

	int red = (int)floor(255 * ((redColor1 / 255.0f) * (redColor2 / 255.0f)));
	int green = (int)floor(255 * ((greenColor1 / 255.0f) * (greenColor2 / 255.0f)));
	int blue = (int)floor(255 * ((blueColor1 / 255.0f) * (blueColor2 / 255.0f)));

	return ChromaAnimationAPI::GetRGB(red, green, blue);
}

int AverageColor(int color1, int color2) {
	return ChromaAnimationAPI::LerpColor(color1, color2, 0.5f);
}

int AddColor(int color1, int color2) {
	int redColor1 = color1 & 0xFF;
	int greenColor1 = (color1 >> 8) & 0xFF;
	int blueColor1 = (color1 >> 16) & 0xFF;

	int redColor2 = color2 & 0xFF;
	int greenColor2 = (color2 >> 8) & 0xFF;
	int blueColor2 = (color2 >> 16) & 0xFF;

	int red = min(redColor1 + redColor2, 255) & 0xFF;
	int green = min(greenColor1 + greenColor2, 255) & 0xFF;
	int blue = min(blueColor1 + blueColor2, 255) & 0xFF;

	return ChromaAnimationAPI::GetRGB(red, green, blue);
}

int SubtractColor(int color1, int color2) {
	int redColor1 = color1 & 0xFF;
	int greenColor1 = (color1 >> 8) & 0xFF;
	int blueColor1 = (color1 >> 16) & 0xFF;

	int redColor2 = color2 & 0xFF;
	int greenColor2 = (color2 >> 8) & 0xFF;
	int blueColor2 = (color2 >> 16) & 0xFF;

	int red = max(redColor1 - redColor2, 0) & 0xFF;
	int green = max(greenColor1 - greenColor2, 0) & 0xFF;
	int blue = max(blueColor1 - blueColor2, 0) & 0xFF;

	return ChromaAnimationAPI::GetRGB(red, green, blue);
}

int MaxColor(int color1, int color2) {
	int redColor1 = color1 & 0xFF;
	int greenColor1 = (color1 >> 8) & 0xFF;
	int blueColor1 = (color1 >> 16) & 0xFF;

	int redColor2 = color2 & 0xFF;
	int greenColor2 = (color2 >> 8) & 0xFF;
	int blueColor2 = (color2 >> 16) & 0xFF;

	int red = max(redColor1, redColor2) & 0xFF;
	int green = max(greenColor1, greenColor2) & 0xFF;
	int blue = max(blueColor1, blueColor2) & 0xFF;

	return ChromaAnimationAPI::GetRGB(red, green, blue);
}

int MinColor(int color1, int color2) {
	int redColor1 = color1 & 0xFF;
	int greenColor1 = (color1 >> 8) & 0xFF;
	int blueColor1 = (color1 >> 16) & 0xFF;

	int redColor2 = color2 & 0xFF;
	int greenColor2 = (color2 >> 8) & 0xFF;
	int blueColor2 = (color2 >> 16) & 0xFF;

	int red = min(redColor1, redColor2) & 0xFF;
	int green = min(greenColor1, greenColor2) & 0xFF;
	int blue = min(blueColor1, blueColor2) & 0xFF;

	return ChromaAnimationAPI::GetRGB(red, green, blue);
}

int InvertColor(int color) {
	int red = 255 - (color & 0xFF);
	int green = 255 - ((color >> 8) & 0xFF);
	int blue = 255 - ((color >> 16) & 0xFF);

	return ChromaAnimationAPI::GetRGB(red, green, blue);
}

int MultiplyNonZeroTargetColorLerp(int color1, int color2, int inputColor) {
	if (inputColor == 0)
	{
		return inputColor;
	}
	float red = (inputColor & 0xFF) / 255.0f;
	float green = ((inputColor & 0xFF00) >> 8) / 255.0f;
	float blue = ((inputColor & 0xFF0000) >> 16) / 255.0f;
	float t = (red + green + blue) / 3.0f;
	return ChromaAnimationAPI::LerpColor(color1, color2, t);
}

int Thresh(int color1, int color2, int inputColor) {
	float red = (inputColor & 0xFF) / 255.0f;
	float green = ((inputColor & 0xFF00) >> 8) / 255.0f;
	float blue = ((inputColor & 0xFF0000) >> 16) / 255.0f;
	float t = (red + green + blue) / 3.0f;
	if (t == 0.0)
	{
		return 0;
	}
	if (t < 0.5)
	{
		return color1;
	}
	else
	{
		return color2;
	}
}


void BlendAnimation1D(const FChromaSDKSceneEffect& effect, FChromaSDKDeviceFrameIndex& deviceFrameIndex, int device, EChromaSDKDevice1DEnum device1d, const char* animationName,
	int* colors, int* tempColors)
{
	const int size = GetColorArraySize1D(device1d);
	const int frameId = deviceFrameIndex._mFrameIndex[device];
	const int frameCount = ChromaAnimationAPI::GetFrameCountName(animationName);
	if (frameId < frameCount)
	{
		//cout << animationName << ": " << (1 + frameId) << " of " << frameCount << endl;
		float duration;
		int animationId = ChromaAnimationAPI::GetAnimation(animationName);
		ChromaAnimationAPI::GetFrame(animationId, frameId, &duration, tempColors, size);
		for (int i = 0; i < size; ++i)
		{
			int color1 = colors[i]; //target
			int tempColor = tempColors[i]; //source

			// BLEND
			int color2;
			switch (effect._mBlend)
			{
			case EChromaSDKSceneBlend::SB_None:
				color2 = tempColor; //source
				break;
			case EChromaSDKSceneBlend::SB_Invert:
				if (tempColor != 0) //source
				{
					color2 = InvertColor(tempColor); //source inverted
				}
				else
				{
					color2 = 0;
				}
				break;
			case EChromaSDKSceneBlend::SB_Threshold:
				color2 = Thresh(effect._mPrimaryColor, effect._mSecondaryColor, tempColor); //source
				break;
			case EChromaSDKSceneBlend::SB_Lerp:
			default:
				color2 = MultiplyNonZeroTargetColorLerp(effect._mPrimaryColor, effect._mSecondaryColor, tempColor); //source
				break;
			}

			// MODE
			switch (effect._mMode)
			{
			case EChromaSDKSceneMode::SM_Max:
				colors[i] = MaxColor(color1, color2);
				break;
			case EChromaSDKSceneMode::SM_Min:
				colors[i] = MinColor(color1, color2);
				break;
			case EChromaSDKSceneMode::SM_Average:
				colors[i] = AverageColor(color1, color2);
				break;
			case EChromaSDKSceneMode::SM_Multiply:
				colors[i] = MultiplyColor(color1, color2);
				break;
			case EChromaSDKSceneMode::SM_Add:
				colors[i] = AddColor(color1, color2);
				break;
			case EChromaSDKSceneMode::SM_Subtract:
				colors[i] = SubtractColor(color1, color2);
				break;
			case EChromaSDKSceneMode::SM_Replace:
			default:
				if (color2 != 0) {
					colors[i] = color2;
				}
				break;
			}
		}
		deviceFrameIndex._mFrameIndex[device] = (frameId + frameCount + effect._mSpeed) % frameCount;
	}
}

void BlendAnimation2D(const FChromaSDKSceneEffect& effect, FChromaSDKDeviceFrameIndex& deviceFrameIndex, int device, EChromaSDKDevice2DEnum device2D, const char* animationName,
	int* colors, int* tempColors)
{
	const int size = GetColorArraySize2D(device2D);
	const int frameId = deviceFrameIndex._mFrameIndex[device];
	const int frameCount = ChromaAnimationAPI::GetFrameCountName(animationName);
	if (frameId < frameCount)
	{
		//cout << animationName << ": " << (1 + frameId) << " of " << frameCount << endl;
		float duration;
		int animationId = ChromaAnimationAPI::GetAnimation(animationName);
		ChromaAnimationAPI::GetFrame(animationId, frameId, &duration, tempColors, size);
		for (int i = 0; i < size; ++i)
		{
			int color1 = colors[i]; //target
			int tempColor = tempColors[i]; //source

			// BLEND
			int color2;
			switch (effect._mBlend)
			{
			case EChromaSDKSceneBlend::SB_None:
				color2 = tempColor; //source
				break;
			case EChromaSDKSceneBlend::SB_Invert:
				if (tempColor != 0) //source
				{
					color2 = InvertColor(tempColor); //source inverted
				}
				else
				{
					color2 = 0;
				}
				break;
			case EChromaSDKSceneBlend::SB_Threshold:
				color2 = Thresh(effect._mPrimaryColor, effect._mSecondaryColor, tempColor); //source
				break;
			case EChromaSDKSceneBlend::SB_Lerp:
			default:
				color2 = MultiplyNonZeroTargetColorLerp(effect._mPrimaryColor, effect._mSecondaryColor, tempColor); //source
				break;
			}

			// MODE
			switch (effect._mMode)
			{
			case EChromaSDKSceneMode::SM_Max:
				colors[i] = MaxColor(color1, color2);
				break;
			case EChromaSDKSceneMode::SM_Min:
				colors[i] = MinColor(color1, color2);
				break;
			case EChromaSDKSceneMode::SM_Average:
				colors[i] = AverageColor(color1, color2);
				break;
			case EChromaSDKSceneMode::SM_Multiply:
				colors[i] = MultiplyColor(color1, color2);
				break;
			case EChromaSDKSceneMode::SM_Add:
				colors[i] = AddColor(color1, color2);
				break;
			case EChromaSDKSceneMode::SM_Subtract:
				colors[i] = SubtractColor(color1, color2);
				break;
			case EChromaSDKSceneMode::SM_Replace:
			default:
				if (color2 != 0) {
					colors[i] = color2;
				}
				break;
			}
		}
		deviceFrameIndex._mFrameIndex[device] = (frameId + frameCount + effect._mSpeed) % frameCount;
	}
}


int keygamecolore[] = 
			{ 0,0,0,
				0,0,0,
				0,0,0 };
void GameLoop()
{
	
	const int sizeKeyboard = GetColorArraySize2D(EChromaSDKDevice2DEnum::DE_Keyboard);
	int* colorsKeyboard = new int[sizeKeyboard];
	int* tempColorsKeyboard = new int[sizeKeyboard];
	

	while (_sWaitForExit)
	{
		// start with a blank frame
		SetStaticColor(colorsKeyboard, _sAmbientColor, sizeKeyboard);
		//KA 
		SetupAnimation2D(ANIMATION_FINAL_KEYBOARD, EChromaSDKDevice2DEnum::DE_Keyboard);

		//Outline playfeeld
		int keys[] = {
			Keyboard::RZKEY::RZKEY_4,
			Keyboard::RZKEY::RZKEY_5,
			Keyboard::RZKEY::RZKEY_6,
			Keyboard::RZKEY::RZKEY_7,
			Keyboard::RZKEY::RZKEY_8,
			Keyboard::RZKEY::RZKEY_R,
			Keyboard::RZKEY::RZKEY_F,
			Keyboard::RZKEY::RZKEY_V,
			Keyboard::RZKEY::RZKEY_I,
			Keyboard::RZKEY::RZKEY_K,
			Keyboard::RZKEY::RZKEY_OEM_9,
			Keyboard::RZKEY::RZKEY_SPACE
		};
		int keysLength = sizeof(keys) / sizeof(int);
		for (int i = 0; i < keysLength; ++i) {
			int color = ChromaAnimationAPI::GetRGB(100, 100, 0);	
			int key = keys[i];
			SetKeyColor(colorsKeyboard, key, color);
		}

		//xo game
		int keysxo[] = {
			Keyboard::RZKEY::RZKEY_T,
			Keyboard::RZKEY::RZKEY_Y,
			Keyboard::RZKEY::RZKEY_U,
			Keyboard::RZKEY::RZKEY_G,
			Keyboard::RZKEY::RZKEY_H,
			Keyboard::RZKEY::RZKEY_J,
			Keyboard::RZKEY::RZKEY_B,
			Keyboard::RZKEY::RZKEY_N,
			Keyboard::RZKEY::RZKEY_M
		};
		keysLength = sizeof(keygamecolore) / sizeof(int);
		for (int i = 0; i < keysLength; ++i) {
			int color = keygamecolore[i];
			int key = keysxo[i];
			SetKeyColor(colorsKeyboard, key, color);
		}
		
		ChromaAnimationAPI::UpdateFrameName(ANIMATION_FINAL_KEYBOARD, 0, 0.1f, colorsKeyboard, sizeKeyboard);
		// display the change
		ChromaAnimationAPI::PreviewFrameName(ANIMATION_FINAL_KEYBOARD, 0);

		Sleep(33); //30 FPS
		
	}
	

	delete[] colorsKeyboard;
	delete[] tempColorsKeyboard;

}

bool turn = false;
void InputHandler()
{
	HandleInput esc = HandleInput(VK_ESCAPE);
	HandleInput inputT = HandleInput('T');
	HandleInput inputZ = HandleInput('Z');
	HandleInput inputU = HandleInput('U');
	HandleInput inputG = HandleInput('G');
	HandleInput inputH = HandleInput('H');
	HandleInput inputJ = HandleInput('J');
	HandleInput inputB = HandleInput('B');
	HandleInput inputN = HandleInput('N');
	HandleInput inputM = HandleInput('M');

	while (_sWaitForExit)
	{
		if (inputT.WasReleased())
		{
			if(turn)
				keygamecolore[0] = ChromaAnimationAPI::GetRGB(0, 255, 0);
			else
				keygamecolore[0] = ChromaAnimationAPI::GetRGB(0, 0, 255);

			turn = !turn;
		}
		if (inputZ.WasReleased())
		{
			if (turn)
				keygamecolore[1] = ChromaAnimationAPI::GetRGB(0, 255, 0);
			else
				keygamecolore[1] = ChromaAnimationAPI::GetRGB(0, 0, 255);

			turn = !turn;
		}
		if (inputU.WasReleased())
		{
			if (turn)
				keygamecolore[2] = ChromaAnimationAPI::GetRGB(0, 255, 0);
			else
				keygamecolore[2] = ChromaAnimationAPI::GetRGB(0, 0, 255);

			turn = !turn;
		}
		if (inputG.WasReleased())
		{
			if (turn)
				keygamecolore[3] = ChromaAnimationAPI::GetRGB(0, 255, 0);
			else
				keygamecolore[3] = ChromaAnimationAPI::GetRGB(0, 0, 255);

			turn = !turn;
		}
		if (inputH.WasReleased())
		{
			if (turn)
				keygamecolore[4] = ChromaAnimationAPI::GetRGB(0, 255, 0);
			else
				keygamecolore[4] = ChromaAnimationAPI::GetRGB(0, 0, 255);

			turn = !turn;
		}
		if (inputJ.WasReleased())
		{
			if (turn)
				keygamecolore[5] = ChromaAnimationAPI::GetRGB(0, 255, 0);
			else
				keygamecolore[5] = ChromaAnimationAPI::GetRGB(0, 0, 255);

			turn = !turn;
		}
		if (inputB.WasReleased())
		{
			if (turn)
				keygamecolore[6] = ChromaAnimationAPI::GetRGB(0, 255, 0);
			else
				keygamecolore[6] = ChromaAnimationAPI::GetRGB(0, 0, 255);

			turn = !turn;
		}
		if (inputN.WasReleased())
		{
			if (turn)
				keygamecolore[7] = ChromaAnimationAPI::GetRGB(0, 255, 0);
			else
				keygamecolore[7] = ChromaAnimationAPI::GetRGB(0, 0, 255);

			turn = !turn;
		}
		if (inputM.WasReleased())
		{
			if (turn)
				keygamecolore[8] = ChromaAnimationAPI::GetRGB(0, 255, 0);
			else
				keygamecolore[8] = ChromaAnimationAPI::GetRGB(0, 0, 255);

			turn = !turn;
		}
		if (esc.WasReleased())
		{
			keygamecolore[0] = ChromaAnimationAPI::GetRGB(0, 0, 0);
			keygamecolore[1] = ChromaAnimationAPI::GetRGB(0, 0, 0);
			keygamecolore[2] = ChromaAnimationAPI::GetRGB(0, 0, 0);
			keygamecolore[3] = ChromaAnimationAPI::GetRGB(0, 0, 0);
			keygamecolore[4] = ChromaAnimationAPI::GetRGB(0, 0, 0);
			keygamecolore[5] = ChromaAnimationAPI::GetRGB(0, 0, 0);
			keygamecolore[6] = ChromaAnimationAPI::GetRGB(0, 0, 0);
			keygamecolore[7] = ChromaAnimationAPI::GetRGB(0, 0, 0);
			keygamecolore[8] = ChromaAnimationAPI::GetRGB(0, 0, 0);

			turn = !turn;
		}

		Sleep(1);
	}
}

void Cleanup()
{
	ChromaAnimationAPI::StopAll();
	ChromaAnimationAPI::CloseAll();
	RZRESULT result = ChromaAnimationAPI::Uninit();
	if (result != RZRESULT_SUCCESS)
	{
		cerr << "Failed to uninitialize Chroma!" << endl;
		exit(1);
	}
}

int main()
{
	fprintf(stdout, "C++ GAME LOOP CHROMA SAMPLE APP\r\n\r\n");

	// setup scene
	/*ÜBERFLÜSSIG*/_sScene = FChromaSDKScene();

	/*
	FChromaSDKSceneEffect effect = FChromaSDKSceneEffect();
	effect._mAnimation = "Animations/Landscape";
	effect._mSpeed = 1;
	effect._mBlend = EChromaSDKSceneBlend::SB_None;
	effect._mState = false;
	effect._mMode = EChromaSDKSceneMode::SM_Add;
	_sScene._mEffects.push_back(effect);
	_sIndexLandscape = (int)_sScene._mEffects.size() - 1;

	effect = FChromaSDKSceneEffect();
	effect._mAnimation = "Animations/Fire";
	effect._mSpeed = 1;
	effect._mBlend = EChromaSDKSceneBlend::SB_None;
	effect._mState = false;
	effect._mMode = EChromaSDKSceneMode::SM_Add;
	_sScene._mEffects.push_back(effect);
	_sIndexFire = (int)_sScene._mEffects.size() - 1;

	effect = FChromaSDKSceneEffect();
	effect._mAnimation = "Animations/Rainbow";
	effect._mSpeed = 1;
	effect._mBlend = EChromaSDKSceneBlend::SB_None;
	effect._mState = false;
	effect._mMode = EChromaSDKSceneMode::SM_Add;
	_sScene._mEffects.push_back(effect);
	_sIndexRainbow = (int)_sScene._mEffects.size() - 1;

	effect = FChromaSDKSceneEffect();
	effect._mAnimation = "Animations/Spiral";
	effect._mSpeed = 1;
	effect._mBlend = EChromaSDKSceneBlend::SB_None;
	effect._mState = false;
	effect._mMode = EChromaSDKSceneMode::SM_Add;
	_sScene._mEffects.push_back(effect);
	_sIndexSpiral = (int)_sScene._mEffects.size() - 1;
	*/
	Init();

	thread thread(GameLoop);
	cout << "Press `ESC` to Quit." << endl;
	cout << "Press `C` to change base color." << endl;
	cout << "Press `A` for ammo/health." << endl;
	cout << "Press `H` to toggle hotkeys." << endl;
	cout << "Press `F` for fire." << endl;
	cout << "Press `L` for landscape." << endl;
	cout << "Press `R` for rainbow." << endl;
	cout << "Press `S` for spiral." << endl;

	InputHandler();

	thread.join();
	Cleanup();
	return 0;
}
