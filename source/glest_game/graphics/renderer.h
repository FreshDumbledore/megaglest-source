// ==============================================================
//	This file is part of Glest (www.glest.org)
//
//	Copyright (C) 2001-2008 Martio Figueroa
//
//	You can redistribute this code and/or modify it under
//	the terms of the GNU General Public License as published
//	by the Free Software Foundation; either version 2 of the
//	License, or (at your option) any later version
// ==============================================================

#ifndef _GLEST_GAME_RENDERER_H_
#define _GLEST_GAME_RENDERER_H_

#include "vec.h"
#include "math_util.h"
#include "model.h"
#include "particle.h"
#include "pixmap.h"
#include "font.h"
#include "matrix.h"
#include "selection.h"
#include "components.h"
#include "texture.h"
#include "model_manager.h"
#include "graphics_factory_gl.h"
#include "font_manager.h"
#include "camera.h"
#include <vector>
#include "model_renderer.h"
#include "model.h"
#include "graphics_interface.h"
#include "base_renderer.h"
#include "simple_threads.h"

#ifdef DEBUG_RENDERING_ENABLED
#	define IF_DEBUG_EDITION(x) x
#	include "debug_renderer.h"
#else
#	define IF_DEBUG_EDITION(x)
#endif

#include "leak_dumper.h"

enum DebugUILevelType {
	debugui_fps 		= 0x01,
	debugui_unit_titles = 0x02
};

namespace Glest{ namespace Game{

using namespace Shared::Graphics;
using namespace Shared::PlatformCommon;

//non shared classes
class Config;
class Game;
class MainMenu;
class Console;
class MenuBackground;
class ChatManager;
class Object;
class ConsoleLineInfo;
class SurfaceCell;

// =====================================================
// 	class MeshCallbackTeamColor
// =====================================================

class MeshCallbackTeamColor: public MeshCallback{
private:
	const Texture *teamTexture;

public:
	void setTeamTexture(const Texture *teamTexture)	{this->teamTexture= teamTexture;}
	virtual void execute(const Mesh *mesh);

	static bool noTeamColors;
};

// ===========================================================
// 	class Renderer
//
///	OpenGL renderer, uses the shared library
// ===========================================================

class VisibleQuadContainerCache {
protected:

	void CopyAll(const VisibleQuadContainerCache &obj) {
		cacheFrame 			= obj.cacheFrame;
		visibleObjectList	= obj.visibleObjectList;
		visibleUnitList		= obj.visibleUnitList;
		visibleQuadUnitList = obj.visibleQuadUnitList;
		visibleScaledCellList = obj.visibleScaledCellList;
		lastVisibleQuad		= obj.lastVisibleQuad;
	}

public:

	VisibleQuadContainerCache() {
		cacheFrame = 0;
		clearCacheData();
	}
	VisibleQuadContainerCache(const VisibleQuadContainerCache &obj) {
		CopyAll(obj);
	}
	VisibleQuadContainerCache & operator=(const VisibleQuadContainerCache &obj) {
		CopyAll(obj);
		return *this;
	}

	void clearCacheData() {
		clearVolatileCacheData();
		clearNonVolatileCacheData();
	}
	void clearVolatileCacheData() {
		visibleUnitList.clear();
		visibleQuadUnitList.clear();
		//inVisibleUnitList.clear();
	}
	void clearNonVolatileCacheData() {
		visibleObjectList.clear();
		visibleScaledCellList.clear();
	}

	int cacheFrame;
	Quad2i lastVisibleQuad;
	std::vector<Object *> visibleObjectList;
	std::vector<Unit   *> visibleQuadUnitList;
	std::vector<Unit   *> visibleUnitList;
	std::vector<Vec2i> visibleScaledCellList;
};

class VisibleQuadContainerVBOCache {
public:
	// Vertex Buffer Object Names
	bool    hasBuiltVBOs;
	uint32	m_nVBOVertices;					// Vertex VBO Name
	uint32	m_nVBOFowTexCoords;				// Texture Coordinate VBO Name for fog of war texture coords
	uint32	m_nVBOSurfaceTexCoords;			// Texture Coordinate VBO Name for surface texture coords
	uint32	m_nVBONormals;					// Normal VBO Name
	//uint32	m_nVBOIndexes;					// Indexes VBO Name
};


class Renderer : public RendererInterface, public BaseRenderer, public SimpleTaskCallbackInterface {
public:
	//progress bar
	static const int maxProgressBar;
	static const Vec4f progressBarBack1;
	static const Vec4f progressBarBack2;
	static const Vec4f progressBarFront1;
	static const Vec4f progressBarFront2;

	//sun and moon
	static const float sunDist;
	static const float moonDist;
	static const float lightAmbFactor;

	//mouse
	static const int maxMouse2dAnim;

	//texture units
	static const GLenum baseTexUnit;
	static const GLenum fowTexUnit;
	static const GLenum shadowTexUnit;

	//selection
	static const float selectionCircleRadius;
	static const float magicCircleRadius;

	//perspective values
	static const float perspFov;
	static const float perspNearPlane;
	static float perspFarPlane;

	//default values
	static const float ambFactor;
	static const Vec4f defSpecularColor;
	static const Vec4f defDiffuseColor;
	static const Vec4f defAmbientColor;
	static const Vec4f defColor;
	static const Vec4f fowColor;

	//light
	static const float maxLightDist;

	static bool renderText3DEnabled;

public:
	enum Shadows {
		sDisabled,
		sProjected,
		sShadowMapping,

		sCount
	};

private:

	//config
	int maxLights;
    bool photoMode;
	int shadowTextureSize;
	int shadowFrameSkip;
	float shadowAlpha;
	bool focusArrows;
	bool textures3D;
	Shadows shadows;
	int maxConsoleLines;

	//game
	const Game *game;
	const MainMenu *menu;

	//misc
	int triangleCount;
	int pointCount;
	Quad2i visibleQuad;
	Vec4f nearestLightPos;
	VisibleQuadContainerCache quadCache;

	//renderers
	ModelRenderer *modelRenderer;
	TextRenderer2D *textRenderer;
	TextRenderer3D *textRenderer3D;
	ParticleRenderer *particleRenderer;

	//texture managers
	ModelManager *modelManager[rsCount];
	TextureManager *textureManager[rsCount];
	FontManager *fontManager[rsCount];
	ParticleManager *particleManager[rsCount];

	//state lists
	GLuint list3d;
	GLuint list2d;
	GLuint list3dMenu;

	//shadows
	GLuint shadowMapHandle;
	Matrix4f shadowMapMatrix;
	int shadowMapFrame;

	//water
	float waterAnim;

	bool allowRenderUnitTitles;
	//std::vector<std::pair<Unit *,Vec3f> > renderUnitTitleList;
	std::vector<Unit *> visibleFrameUnitList;
	string visibleFrameUnitListCameraKey;

	bool no2DMouseRendering;
	bool showDebugUI;
	int showDebugUILevel;

	int lastRenderFps;
	float smoothedRenderFps;
	bool shadowsOffDueToMinRender;

	SimpleTaskThread *saveScreenShotThread;
	Mutex saveScreenShotThreadAccessor;
	std::list<std::pair<string,Pixmap2D *> > saveScreenQueue;

	//std::map<Vec3f,Vec3f> worldToScreenPosCache;

	std::map<uint32,VisibleQuadContainerVBOCache > mapSurfaceVBOCache;

	class SurfaceData {
	public:
		SurfaceData(){};
		static uint32 nextUniqueId;
		uint32 uniqueId;
		int bufferCount;
		int textureHandle;
		vector<Vec2f> texCoords;
		vector<Vec2f> texCoordsSurface;
		vector<Vec3f> vertices;
		vector<Vec3f> normals;
	};

	VisibleQuadContainerVBOCache * GetSurfaceVBOs(SurfaceData *cellData);
	void ReleaseSurfaceVBOs();
	std::map<string,std::pair<Chrono, std::vector<SurfaceData> > > mapSurfaceData;
	static bool rendererEnded;
private:
	Renderer();
	~Renderer();

public:
	static Renderer &getInstance();
	static bool isEnded();

	void reinitAll();

    //init
	void init();
	void initGame(const Game *game);
	void initMenu(const MainMenu *mm);
	void reset3d();
	void reset2d();
	void reset3dMenu();

	//end
	void end();
	void endMenu();
	void endGame();

	//get
	int getTriangleCount() const	{return triangleCount;}
	int getPointCount() const		{return pointCount;}

	//misc
	void reloadResources();

	//engine interface
	void initTexture(ResourceScope rs, Texture *texture);
	void endTexture(ResourceScope rs, Texture *texture,bool mustExistInList=false);
	void endLastTexture(ResourceScope rs, bool mustExistInList=false);

	Model *newModel(ResourceScope rs);
	void endModel(ResourceScope rs, Model *model, bool mustExistInList=false);
	void endLastModel(ResourceScope rs, bool mustExistInList=false);

	Texture2D *newTexture2D(ResourceScope rs);
	Texture3D *newTexture3D(ResourceScope rs);
	Font2D *newFont(ResourceScope rs);
	Font3D *newFont3D(ResourceScope rs);

	TextRenderer2D *getTextRenderer() const	{return textRenderer;}
	TextRenderer3D *getTextRenderer3D() const	{return textRenderer3D;}

	void manageParticleSystem(ParticleSystem *particleSystem, ResourceScope rs);
	void cleanupParticleSystems(vector<ParticleSystem *> &particleSystems,ResourceScope rs);
	void cleanupUnitParticleSystems(vector<UnitParticleSystem *> &particleSystems,ResourceScope rs);
	bool validateParticleSystemStillExists(ParticleSystem * particleSystem,ResourceScope rs) const;
	void updateParticleManager(ResourceScope rs,int renderFps=-1);
	void renderParticleManager(ResourceScope rs);
	void swapBuffers();

    //lights and camera
	void setupLighting();
	void loadGameCameraMatrix();
	void loadCameraMatrix(const Camera *camera);
	void computeVisibleQuad();

    //basic rendering
	void renderMouse2d(int mouseX, int mouseY, int anim, float fade= 0.f);
    void renderMouse3d();
    void renderBackground(const Texture2D *texture);
	void renderTextureQuad(int x, int y, int w, int h, const Texture2D *texture, float alpha=1.f,const Vec3f *color=NULL);
	void renderConsole(const Console *console, const bool showAll=false, const bool showMenuConsole=false, int overrideMaxConsoleLines=-1);
	void renderConsoleLine3D(int lineIndex, int xPosition, int yPosition, int lineHeight, Font3D* font, string stringToHightlight, const ConsoleLineInfo *lineInfo);
	void renderConsoleLine(int lineIndex, int xPosition, int yPosition, int lineHeight, Font2D* font, string stringToHightlight, const ConsoleLineInfo *lineInfo);

	void renderChatManager(const ChatManager *chatManager);
	void renderResourceStatus();
	void renderSelectionQuad();
	void renderText(const string &text, Font2D *font, float alpha, int x, int y, bool centered= false);
	void renderText(const string &text, Font2D *font, const Vec3f &color, int x, int y, bool centered= false);
	void renderText(const string &text, Font2D *font, const Vec4f &color, int x, int y, bool centered=false);
	void renderTextShadow(const string &text, Font2D *font,const Vec4f &color, int x, int y, bool centered= false);

	void renderText3D(const string &text, Font3D *font, float alpha, int x, int y, bool centered);
	void renderText3D(const string &text, Font3D *font, const Vec3f &color, int x, int y, bool centered);
	void renderText3D(const string &text, Font3D *font, const Vec4f &color, int x, int y, bool centered);
	void renderTextShadow3D(const string &text, Font3D *font,const Vec4f &color, int x, int y, bool centered=false);
	void renderProgressBar3D(int size, int x, int y, Font3D *font, int customWidth=-1, string prefixLabel="", bool centeredText=true);

	Vec2f getCentered3DPos(const string &text, Font3D *font, Vec2f &pos, int w, int h, bool centeredW, bool centeredH);
	void renderTextBoundingBox3D(const string &text, Font3D *font, const Vec4f &color, int x, int y, int w, int h, bool centeredW, bool centeredH);
	void renderTextBoundingBox3D(const string &text, Font3D *font, const Vec3f &color, int x, int y, int w, int h, bool centeredW, bool centeredH);
	void renderTextBoundingBox3D(const string &text, Font3D *font, float alpha, int x, int y, int w, int h, bool centeredW, bool centeredH);

	void beginRenderToTexture(Texture2D **renderToTexture);
	void endRenderToTexture(Texture2D **renderToTexture);

	void renderFPSWhenEnabled(int lastFps);

    //components
	void renderLabel(GraphicLabel *label);
	void renderLabel(GraphicLabel *label,const Vec3f *color);
	void renderLabel(GraphicLabel *label,const Vec4f *color);
    void renderButton(GraphicButton *button,const Vec4f *fontColorOverride=NULL,bool *lightedOverride=NULL);
    void renderCheckBox(const GraphicCheckBox *box);
    void renderLine(const GraphicLine *line);
    void renderScrollBar(const GraphicScrollBar *sb);
    void renderListBox(GraphicListBox *listBox);
	void renderMessageBox(GraphicMessageBox *listBox);

    //complex rendering
    void renderSurface(const int renderFps);
	void renderObjects(const int renderFps);

	void renderWater();
    void renderUnits(const int renderFps);

	void renderSelectionEffects();
	void renderWaterEffects();
	void renderHud();
	void renderMinimap();
    void renderDisplay();
	void renderMenuBackground(const MenuBackground *menuBackground);
	void renderMapPreview(const MapPreview *map, bool renderAll, int screenX, int screenY,Texture2D **renderToTexture=NULL);

	//computing
    bool computePosition(const Vec2i &screenPos, Vec2i &worldPos);
	void computeSelected(Selection::UnitContainer &units, const Object *&obj, const bool withObjectSelection, const Vec2i &posDown, const Vec2i &posUp);

    //gl wrap
	string getGlInfo();
	string getGlMoreInfo();
	void autoConfig();

	//clear
    void clearBuffers();
	void clearZBuffer();

	//shadows
	void renderShadowsToTexture(const int renderFps);

	//misc
	void loadConfig();
	void saveScreen(const string &path);
	Quad2i getVisibleQuad() const		{return visibleQuad;}

	//static
	static Shadows strToShadows(const string &s);
	static string shadowsToStr(Shadows shadows);

	const Game * getGame() { return game; }

	void setAllowRenderUnitTitles(bool value);
	bool getAllowRenderUnitTitles() { return allowRenderUnitTitles; }
	void renderUnitTitles(Font2D *font, Vec3f color);
	void renderUnitTitles3D(Font3D *font, Vec3f color);
	Vec3f computeScreenPosition(const Vec3f &worldPos);

	void setPhotoMode(bool value) { photoMode = value; }

	bool getNo2DMouseRendering() const { return no2DMouseRendering; }
	void setNo2DMouseRendering(bool value) { no2DMouseRendering = value; }

	bool getShowDebugUI() const { return showDebugUI; }
	void setShowDebugUI(bool value) { showDebugUI = value; }

	int getShowDebugUILevel() const { return showDebugUILevel; }
	void setShowDebugUILevel(int value) { showDebugUILevel=value; }
	void cycleShowDebugUILevel();

	void setLastRenderFps(int value);
	int getLastRenderFps() const { return lastRenderFps;}

	VisibleQuadContainerCache & getQuadCache(bool updateOnDirtyFrame=true,bool forceNew=false);
	void removeObjectFromQuadCache(const Object *o);
	void removeUnitFromQuadCache(const Unit *unit);

	uint64 getCurrentPixelByteCount(ResourceScope rs=rsGame) const;
	unsigned int getSaveScreenQueueSize();

	Texture2D *saveScreenToTexture(int x, int y, int width, int height);

	void renderProgressBar(int size, int x, int y, Font2D *font,int customWidth=-1, string prefixLabel="", bool centeredText=true);

	static Texture2D * findFactionLogoTexture(string logoFilename);
	static Texture2D * preloadTexture(string logoFilename);
	int getCachedSurfaceDataSize() const { return mapSurfaceData.size(); }

private:
	//private misc
	float computeSunAngle(float time);
	float computeMoonAngle(float time);
	Vec4f computeSunPos(float time);
	Vec4f computeMoonPos(float time);
	Vec3f computeLightColor(float time);
	Vec4f computeWaterColor(float waterLevel, float cellHeight);
	void checkExtension(const string &extension, const string &msg);

	//selection render
	void renderObjectsFast(bool renderingShadows = false, bool resourceOnly = false);
	void renderUnitsFast(bool renderingShadows = false);

	//gl requirements
	void checkGlCaps();
	void checkGlOptionalCaps();

	//gl init
	void init3dList();
    void init2dList();
	void init3dListMenu(const MainMenu *mm);

	//misc
	void loadProjectionMatrix();
	void enableProjectiveTexturing();

	//private aux drawing
	void renderSelectionCircle(Vec3f v, int size, float radius);
	void renderArrow(const Vec3f &pos1, const Vec3f &pos2, const Vec3f &color, float width);
	void renderTile(const Vec2i &pos);
	void renderQuad(int x, int y, int w, int h, const Texture2D *texture);

	void simpleTask(BaseThread *callingThread);

	//static
    static Texture2D::Filter strToTextureFilter(const string &s);
    void cleanupScreenshotThread();

};

}} //end namespace

#endif
