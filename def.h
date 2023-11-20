/*
todo
- dropdowns 
- entity "system"
- entity definitions in editor
- pathing and routes
- colliders
*/

#pragma warning (disable: 4244)
#pragma warning (default: 4255) // missing prototypes
#pragma warning (disable: 4142)

#define assert(exp) (exp ? (exp) : (*(int*)0 = 0))
//#define array_size(arr) (assert(*((int*)arr-2) > 0) ? *((int*)arr-2) : 0)
/*(int)((byte*)&arr##_size - (byte*)&arr)*/
/*(sizeof(arr)/sizeof(arr[0]))*/
#define array_count(arr) asd // todo: is the struct packing ok with this in 64bit?
//#define array_define(type, name, size) int name##_count; 
//type name[64]; 
//int name##_size
//#define array_init(arr) arr##_size = (sizeof(arr)/sizeof(arr[0]))
#define c_array_size(a) (sizeof(a)/sizeof(a[0]))

#define KILOBYTES(n) (n*1024)
#define MEGABYTES(n) (n*1024*1024)
#define GIGABYTES(n) (n*1024*1024*1024)

//#define _MATH_DECLARATIONS
#include "../core/math.c"
#include "../core/memory.c"

typedef struct {
	GLuint handle;
	float width;
	float height;
} r_texture;
typedef struct {
	r_texture texture;
	vec2 uv;
	vec2 uv2;
	vec2 pixelSize;
} r_sprite;

#define _UI_HEADER
#include "../core/ui.c"

//#define _GFX_HEADER
#include "../core/gfx.c"

#include "quests.c"

typedef __int64  int64_t;
typedef unsigned __int64  uint64_t;
typedef __int32  int32_t;
typedef unsigned __int32  uint32_t;
typedef __int16  int16_t;
typedef unsigned __int16  uint16_t;
typedef __int64  s64;
typedef unsigned __int64  u64;
typedef __int32  s32;
typedef unsigned __int32  u32;
typedef __int16  s16;
typedef unsigned __int16  u16;
typedef u32 b32;
typedef unsigned char byte; // TODO remove all duplications
#define TRUE 1
#define FALSE 0

// Platform
#define KEY_F1 0x70
#define KEY_F2 0x71
#define KEY_F3 0x72
#define KEY_F4 0x73
#define KEY_F5 0x74
#define KEY_F6 0x75
#define KEY_F7 0x76
#define KEY_F8 0x77
#define KEY_F9 0x78
#define KEY_F10 0x79
#define KEY_F11 0x7A
#define KEY_F12 0x7B
#define KEY_LEFT 0x25
#define KEY_UP 0x26
#define KEY_RIGHT 0x27
#define KEY_DOWN 0x28

typedef void* file_handle;
typedef struct {
	uint64_t created;
	uint64_t modified;
	int size; // 32bit only
} file_info;

#define openFilePROC(name) file_handle name(char* path)
#define readFilePROC(name) int name(file_handle file, int offset, int size, void* output)
#define readFileStreamPROC(name) int name(file_handle file, int size, void* output)
#define writeFilePROC(name) void name(file_handle file, int offset, int size, void* data)
#define writeFileStreamPROC(name) void name(file_handle file, int size, void* data)
#define closeFilePROC(name) void name(file_handle file)
#define fileStatPROC(name) file_info name(file_handle file)
#define fileStatByPathPROC(name) file_info name(char* path)
/*typedef openFilePROC(openFile_proctype);
typedef readFilePROC(readFile_proctype);
typedef readFileStreamPROC(readFileStream_proctype);
typedef writeFilePROC(writeFile_proctype);
typedef closeFilePROC(closeFile_proctype);
typedef fileStatPROC(fileStat_proctype);
typedef fileStatByPathPROC(fileStatByPath_proctype);*/
#define PLATFORM_FUNCTIONS \
PLATFORM_FUNCTION(openFile)\
PLATFORM_FUNCTION(readFile)\
PLATFORM_FUNCTION(readFileStream)\
PLATFORM_FUNCTION(writeFile)\
PLATFORM_FUNCTION(writeFileStream)\
PLATFORM_FUNCTION(closeFile)\
PLATFORM_FUNCTION(fileStat)\
PLATFORM_FUNCTION(fileStatByPath)

#define PLATFORM_FUNCTION(name) typedef name##PROC(name##_proctype);
PLATFORM_FUNCTIONS
#undef PLATFORM_FUNCTION

typedef struct {
	int width;
	int height;
	
	byte keyboard[256];
	char text[8];
	point mouse;
	int lbuttonDownEvent;
	int rbuttonDownEvent;
	int lbuttonUpEvent;
	int rbuttonUpEvent;
	int lbuttonIsDown;
	int rbuttonIsDown;
	
	double runtimeInSeconds;
} win_state;


// Game
#define WORLD_SIZE 32

typedef enum {
	TILE_DOOR = 1, // +1 from spritesheet to allow for 0
	TILE_WINDOW = 25,
	TILE_FENCE = 27,
} TILE_TYPE;
typedef struct {
	int type;
	b32 hasEntity;
	int entityIndex;
	u32 _ignored1;
	u32 _ignored2;
	u32 _ignored3;
	u32 _ignored4;
} tile;
typedef struct {
	vec3 pos; // location of the bottom left corner
	vec3 size;
	char name[16];
	u32 _ignored1;
	u32 _ignored2;
	u32 _ignored3;
	u32 _ignored4;
} building;

typedef enum {
	ENTITY_ACTOR,
	ENTITY_ITEM,
} entity_type;
char* entityTypeNames[] = {
	"Actor",
	"Item"
};
typedef struct {
	entity_type type;
	char name[16];
	vec3 pos;
	vec3 route[4];
	int numRoutes;
	int sprite;
	int inDialog;
	int animation;
	u32 _ignored1;
	u32 _ignored2;
	u32 _ignored3;
	u32 _ignored4;
	u32 _ignored5;
	u32 _ignored6;
	u32 _ignored7;
	u32 _ignored8;
} entity;

#define MAX_TILES (WORLD_SIZE*WORLD_SIZE)
#define MAX_BUILDINGS 32
#define MAX_ENTITIES 64
typedef struct {
	char code[8];
	int tileCount;
	int buildingCount;
	int entityCount;
} game_level_header;
typedef struct {
	game_level_header;
	tile tiles[MAX_TILES];
	//int nEntities;
	//int buildings_size;
	//int nBuildings; // todo: 64bit?
	building buildings[MAX_BUILDINGS];
	entity entities[MAX_ENTITIES];
} game_level;
typedef struct {
	//array_define(entity, entities, 64);
	entity entities[MAX_ENTITIES];
} game_world;

char* editorModeNames[] = {
	"Select",
	"Building",
	"Tile",
	"Entity",
};

typedef struct {
	memory_arena memory;
	memory_arena generalMemory;
	memory_arena stringBuffer;
	
	float time;
	float camx;
	float camy;
	
	game_level level;
	game_world world;
	
	struct {
		int showUI;
	} debug;
	
	struct {
		//int mx;
		//int my;
		//int dragx;
		//int dragy;
		TILE_TYPE tile;
		b32 showGrid;
		enum {
			EDITOR_MODE_SELECT,
			EDITOR_MODE_BUILDING,
			EDITOR_MODE_TILE,
			EDITOR_MODE_ENTITY,
		} mode;
		//int selectedTile;
		vec3 select;
		vec3 selectEnd;
		vec3 insert;
		vec3 insertEnd;
		entity editEntity;
		b32 chooseSprite;
	} editor;
	
	int mode;
	
	gfx_state gfx;
	r_texture tileTex;
	r_texture cabinTex;
	r_texture charactersTexture;
	gfx_program* snowShader;
	gfx_program* tileShader;
	gfx_program* uiBoxShader;
	r_texture fontTexture;
	ui_font font;
	ui_state ui;
	
	vec2 playerPos;
	float playerAniFrame;
	int playerFacingDir; // note: 0=left, 1=right
	
	game_dialog dialogs;
} game_state;

__declspec(dllexport) game_state* game;
__declspec(dllexport) win_state* platform;
vec4 BLUE = {0.1f, 0.2f, 0.5f, 1};
vec4 ORANGE = {1.0f, 0.4f, 0.2f, 1};
vec4 PURPLE = {1.0f, 0.4f, 1.2f, 1};

#include "renderer.c"
#define _UI_IMPLEMENTATION
#include "../core/ui.c"

#define GAMELOOP_PROC(name) __declspec(dllexport) void name(win_state* win, game_state* game, float dt)
#define GAMESTART_PROC(name) __declspec(dllexport) void name(win_state* win, game_state* game)
#define SETUP_EDITOR_PROC(name) void name(HWND window)
typedef GAMESTART_PROC(gamestart_proc);
typedef GAMELOOP_PROC(gameloop_proc);
typedef SETUP_EDITOR_PROC(setupEditor_proc);
//GAMESTART_PROC(gamestart);
//GAMELOOP_PROC(gameloop);
//SETUP_EDITOR_PROC(setupEditor);

void editorloop(win_state* win, game_state* game);

void saveLevelFile(game_level* level);
//point worldCoordsToStorageCoords(point coords);