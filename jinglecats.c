#define WIN32_LEAN_AND_MEAN
#include <WINDOWS.H>
#include <gl/GL.H>
#include <STDLIB.H>
#include <STDIO.H>

#include "def.h"

/*__declspec(dllexport) openFile_proc* openFile;
__declspec(dllexport) readFile_proc* readFile;
__declspec(dllexport) readFileStream_proc* readFileStream;
__declspec(dllexport) writeFile_proc* writeFile;
__declspec(dllexport) fileStat_proc* fileStat;
__declspec(dllexport) closeFile_proc* closeFile;
__declspec(dllexport) fileStatByPath_proc* fileStatByPath;*/
#define PLATFORM_FUNCTION(name) __declspec(dllexport) name##_proctype* name;
PLATFORM_FUNCTIONS
#undef PLATFORM_FUNCTION

int keyIsDown(win_state* p, byte key) {
	return p->keyboard[key] & 1;
}

int keyUpEvent(win_state* p, byte key) {
	return (p->keyboard[key] & 3) == 2;
}

int keyDownEvent(win_state* p, byte key) {
	return (p->keyboard[key] & 3) == 1;
}

#pragma pack(push, 1)
typedef struct {
	char header[2];
	uint32_t size;
	uint16_t reserved1;
	uint16_t reserved2;
	uint32_t offset;
	
	// Windows BITMAPINFOHEADER
	uint32_t headerSize;
	int32_t bitmapWidth;
	int32_t bitmapHeight;
	uint16_t colorPlanes;
	uint16_t colorDepth;
	uint32_t compression;
	uint32_t imageSize;
	int32_t hres;
	int32_t vres;
	uint32_t paletteSize;
	uint32_t importantColors;
} bmp_header;
#pragma pack(pop)

typedef struct {
	uint32_t* data;
	bmp_header* header;
} bmp;

bmp loadBmp(char* filename) {
	FILE* fontFile;
	long fileSize;
	void* fontData;
	bmp_header* header;
	uint32_t* palette;
	char* data;
	int rowSize;
	uint32_t* image;
	bmp result;

	fontFile = fopen(filename, "r"); // todo: this stuff crashes when file not found
	if(!fontFile) {
		printf("cant open file\n");
	}
	fseek(fontFile, 0, SEEK_END);
	fileSize = ftell(fontFile);
	fontData = malloc(fileSize);
	rewind(fontFile);
	fread(fontData, 1, fileSize, fontFile);
	fclose(fontFile);
	
	header = (bmp_header*)fontData;
	palette = (uint32_t*)((char*)fontData+14+header->headerSize);
	data = (char*)fontData+header->offset;
	rowSize = ((header->colorDepth*header->bitmapWidth+31) / 32) * 4;
	
	image = (uint32_t*)malloc(sizeof(uint32_t)*header->bitmapWidth*header->bitmapHeight);
	//{for(int w=0; w<header.bitmapHeight}
	{
		int row;
		int pixel;
		for(row=0; row<header->bitmapHeight; ++row) {
			int bitIndex=0;
			//printf("row %i \n", row);
// 			if(row==255) {
// 				DebugBreak();
// 			}
			for(pixel=0; pixel<header->bitmapWidth; ++pixel) {//while((bitIndex/8) < rowSize) {
				uint32_t* chunk = (uint32_t*)((char*)fontData+header->offset+(row*rowSize)+(bitIndex/8));
				uint32_t pi = *chunk;
				if(header->colorDepth<8) {
					pi >>= (header->colorDepth-(bitIndex%8));
				}
				pi &= (((int64_t)1<<header->colorDepth)-1);
				if(header->colorDepth>8) {
					image[row*header->bitmapWidth+pixel] = pi;
				} else {
					image[row*header->bitmapWidth+pixel] = palette[pi];
				}
				if(/*image[row*header->bitmapWidth+pixel]==0xFF000000 ||*/
				   image[row*header->bitmapWidth+pixel]==0xFFFF00FF) {
					image[row*header->bitmapWidth+pixel] = 0;
				}
// 				if(pixel==120) {
// 					int asd = 0;
// 				}
				bitIndex += header->colorDepth;
			}
		}
	}
	
	result.data = image;
	result.header = header;
	return result;
}

r_texture createTexture(bmp image) {
	r_texture result;
	result.width = image.header->bitmapWidth;
	result.height = image.header->bitmapHeight;
	glGenTextures(1, &result.handle);
	glBindTexture(GL_TEXTURE_2D, result.handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.header->bitmapWidth, image.header->bitmapHeight, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, image.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	return result;
}

#define LEVEL_FORMAT_VERSION 3
void saveLevelFile(game_level* level) {
	file_handle file = openFile("level.dat");
	if(file) {
		// note: the stack goes backwards so this doesn't work
		/*char header[4] = "LEVL";
		int32_t version = LEVEL_FORMAT_VERSION;*/
		
		//char header[8] = "LEVL";
		//*(u32*)(header+4) = LEVEL_FORMAT_VERSION;
		game_level_header header = *(game_level_header*)level;
		copyMemory(header.code, "LEVL", 4);
		u32 version = LEVEL_FORMAT_VERSION;
		copyMemory(header.code+4, &version, 4);
		
		writeFileStream(file, sizeof(header), &header);
		writeFileStream(file, sizeof(tile)*header.tileCount, level->tiles);
		writeFileStream(file, sizeof(building)*header.buildingCount, level->buildings);
		writeFileStream(file, sizeof(entity)*header.entityCount, level->entities);
		SetEndOfFile(file);
		closeFile(file);
	}
}
void loadLevelFile(game_level* level) {
	level->tileCount = MAX_TILES;
	file_handle file = openFile("level.dat");
	if(file) {
		/*char levl[4] = "LEVL";
		char header[8];
		readFile(file, 0, 8, header);
		if(*(uint32_t*)(header+4) == 1 &&
		   LEVEL_FORMAT_VERSION == 2) {
			readFile(file, 8, sizeof(level->tiles), level);
			int offset = 8 + sizeof(level->tiles);
			readFile(file, offset, 64*sizeof(int), level);
			offset += 64*sizeof(int);
			readFile(file, offset, sizeof(int)*2 + sizeof(level->buildings), level);
			closeFile(file);
		} else {
			file_info info = fileStat(file);
			assert(info.size == sizeof(game_level)+8);
			if(*(uint32_t*)header == *(uint32_t*)levl &&
			   *(uint32_t*)(header+4) == LEVEL_FORMAT_VERSION) {
				readFile(file, 8, sizeof(game_level), level);
			}
			closeFile(file);
		}*/
		
		char levl[4] = "LEVL";
		//char code[8];
		//readFile(file, 0, 8, code);
		//game_level_header header;
		readFileStream(file, sizeof(game_level_header), level);
		if(*(u32*)level->code == *(u32*)levl) {
			if(*(u32*)(level->code+4) != LEVEL_FORMAT_VERSION) {
				uiMessage("Level file version mismatch");
			}
			//*(game_level_header*)&game->level = header;
			readFileStream(file, sizeof(tile)*level->tileCount, level->tiles);
			readFileStream(file, sizeof(building)*level->buildingCount, level->buildings);
			readFileStream(file, sizeof(entity)*level->entityCount, level->entities);
			
			copyMemory(game->world.entities,
					   game->level.entities,
					   sizeof(entity)*game->level.entityCount);
		}
		closeFile(file);
	}
}

//int tiles[8*5];

// #define GL_FRAGMENT_SHADER                0x8B30
// #define GL_VERTEX_SHADER                  0x8B31
// #define GL_GEOMETRY_SHADER                0x8DD9
//extern GLuint glCreateShader(GLenum type);
// typedef GLuint (APIENTRY *CREATE_SHADER)(GLenum type);
// CREATE_SHADER glCreateShader;

/*point worldCoordsToStorageCoords(point coords) {
	return _point(coords.x+(WORLD_SIZE/2), coords.y+(WORLD_SIZE/2));
}

vec2 storageCoordsToWorldCoords(vec2 coords) {
	return _vec2(coords.x-(WORLD_SIZE/2), coords.y-(WORLD_SIZE/2));
}*/

tile* getTile(vec3 pos) {
	int x = pos.x+0.5f + (WORLD_SIZE/2);
	int y = pos.y+0.5f + (WORLD_SIZE/2);
	return &game->level.tiles[y*WORLD_SIZE+x];
}

gfx_file_info gfxFileStat(char* path) {
	file_info info = fileStatByPath(path);
	gfx_file_info result;
	result.writeTime = info.modified;
	result.size = info.size;
	return result;
}

int gfxLoadFile(char* path, int size, void* output) {
	file_handle file = openFile(path);
	if(file) {
		if(readFile(file, 0, size, output)) {
			closeFile(file);
			return 1;			
		}
		closeFile(file);
	}
	return 0;
}

GAMESTART_PROC(gamestart) {
	ui = &game->ui;
	
	{
		game->gfx.memorySize = KILOBYTES(90);
		game->gfx.memory = pushMemory(&game->memory, game->gfx.memorySize);
		
		game->gfx.transient.size = KILOBYTES(8);
		game->gfx.transient.address = pushMemory(&game->memory, game->gfx.transient.size);
		
		game->gfx.programMemory.size = KILOBYTES(1);
		game->gfx.programMemory.address = pushMemory(&game->memory, game->gfx.programMemory.size);
		
		game->gfx.shaderLinks.size = KILOBYTES(1);
		game->gfx.shaderLinks.address = pushMemory(&game->memory, game->gfx.shaderLinks.size);
		
		game->gfx.freeBlocks = game->gfx.memory;
		game->gfx.freeBlocks->size = game->gfx.memorySize;
		
		game->ui.widgetTree.size = KILOBYTES(10);
		game->ui.widgetTree.address = pushMemory(&game->memory, game->ui.widgetTree.size);
		game->ui.widgetState.size = KILOBYTES(10);
		game->ui.widgetState.address = pushMemory(&game->memory, game->ui.widgetState.size);
		game->ui.msgMemory.size = KILOBYTES(10);
		game->ui.msgMemory.address = pushMemory(&game->memory, game->ui.msgMemory.size);
		game->ui.transient.size = KILOBYTES(10);
		game->ui.transient.address = pushMemory(&game->memory, game->ui.transient.size);
		
		game->generalMemory.size = KILOBYTES(10);
		game->generalMemory.address = pushMemory(&game->memory, game->generalMemory.size);
		
		game->stringBuffer.size = KILOBYTES(10);
		game->stringBuffer.address = pushMemory(&game->memory, game->stringBuffer.size);
	}
	
	bmp tileBmp = loadBmp("tiles2.bmp");
	bmp cabinBmp = loadBmp("cabins.bmp");
	game->tileTex = createTexture(tileBmp);
	game->cabinTex = createTexture(cabinBmp);
	
	bmp charactersBmp = loadBmp("characters.bmp");
	game->charactersTexture = createTexture(charactersBmp);
	
	bmp fontBmp = loadBmp("font_v3.bmp");
	for(int i=0; i<fontBmp.header->bitmapWidth*fontBmp.header->bitmapHeight; ++i) {
		if(fontBmp.data[i] == 0xFFFFFFFF) {
			fontBmp.data[i] = 0x0;
		}
	}
	for(int c=0; c<128; ++c) {
		int x = c%16 * (8);
		int y = c/(16) * 8;
		for(int p=0; p<64; ++p) {
			int* pixel = &fontBmp.data[(y+(p/8))*(8*16) + x+(p%8)];
			byte horizontal = game->font.kerning[c] & 0x0f;
			byte verticle = (game->font.kerning[c] & 0xf0)>>4;
			if(*pixel == 0xFFFF0000) {
				game->font.kerning[c] = (verticle<<4) | ++horizontal;
				*pixel = 0;
			}
			if(*pixel == 0xFF00FF00) {
				game->font.kerning[c] = (++verticle<<4) | horizontal;
				*pixel = 0;
			}
			if(*pixel == 0xFF000000) {
				*pixel = 0xFFFFFFFF;				
			}
		}
	}
	for(int c=0; c<128; ++c) {
		//game->font.kerning[c] = '0' + game->font.kerning[c];
	}
	game->fontTexture = createTexture(fontBmp);
	game->font.texture = &game->fontTexture;

// 	glCreateShader = (CREATE_SHADER)wglGetProcAddress("glCreateShader");
// 	shader = glCreateShader(GL_FRAGMENT_SHADER);

	//{int i; for(i=0; i<array_size(world); ++i){
		//world[i].index = 1+(rand()%16);
	//}}

	load_opengl_extensions(&game->gfx);
	set_gfx_globals(&game->gfx);
	gfx_set_file_stat_callback(gfxFileStat);
	gfx_set_load_file_callback(gfxLoadFile);
	game->snowShader = gfx_create_shader("basic.vert", "snow.frag");
	game->tileShader = gfx_create_shader("basic.vert", "tile.frag");
	game->uiBoxShader = gfx_create_shader("basic.vert", "ui_box.gl.c");
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	file_handle gif = openFile("cats.gif");
	file_info gifInfo = fileStat(gif);
	void* gifData = malloc(gifInfo.size);
	readFile(gif, 0, gifInfo.size, gifData);
	closeFile(gif);
	
	/*file_handle file = win->openFile("level.dat");
	if(file) {
		file_info info = win->fileStat(file);
		void* data = malloc(info.size);
		win->readFile(file, 0, info.size, data);
		win->closeFile(file);
	}*/
	
	loadLevelFile(&game->level);
	
	game->mode = 1;
	
	//game->dialogs = initGameDialog();
	int x = 0;
	game->dialogs.santaIntro.count = sizeof(testLines) / sizeof(testLines[0]);
	game->dialogs.santaIntro.lines = 
		pushAndCopyMemory(&game->generalMemory, testLines, sizeof(testLines));
}

/*void drawTile(int index, float x, float y) {
	float ts = 0.125;

	gfx_uf2("world_position", x, y);
	
	float z = 0;//0.0f+(y)/100.0f;
	glBegin(GL_QUADS);
	glTexCoord2f(0+((float)(index%8)*ts), 1-0.125f-((float)(index/8)*ts));
	glVertex3f(-0.5, -0.5, z);
	glTexCoord2f(ts+((float)(index%8)*ts), 1-0.125f-((float)(index/8)*ts));
	glVertex3f(0.5,  -0.5, z);
	glTexCoord2f(ts+((float)(index%8)*ts), 1-((float)(index/8)*ts));
	glVertex3f(0.5,  0.5, z);
	glTexCoord2f(0+((float)(index%8)*ts), 1-((float)(index/8)*ts));
	glVertex3f(-0.5, 0.5, z);
	glEnd();
}*/

//point positionToTileStorageCoords(vec2 pos) {
	//return worldCoordsToStorageCoords(_point(floorf(pos.x+0.5f), floorf(pos.y+0.5f)));
//}

GAMELOOP_PROC(gameloop) {
	set_gfx_globals(&game->gfx);
	gfx_set_file_stat_callback(gfxFileStat);
	gfx_set_load_file_callback(gfxLoadFile);
	
	strBuffer(&game->stringBuffer);
	
	ui = &game->ui;
	ui->scale = _vec2(2.0f/(float)win->width * 8.0f*2.0f, 2.0f/(float)win->height * 8.0f*2.0f);
	ui->screenSize = div2(_vec2(2,2), ui->scale);
	ui->mousePos = _vec2(win->mouse.x/16.0f, win->mouse.y/16.0f);
	ui->lbuttonIsDown = win->lbuttonIsDown;
	ui->input = win->text;
	ui->leftArrow = keyDownEvent(win, KEY_LEFT);
	ui->rightArrow = keyDownEvent(win, KEY_RIGHT);
	ui->upArrow = keyDownEvent(win, KEY_UP);
	ui->downArrow = keyDownEvent(win, KEY_DOWN);
	
	
	int h = WORLD_SIZE/2;
	// printf("%i %i \n", mx, my);
	
	if(keyUpEvent(win, KEY_F1)) {
		game->mode = !game->mode;
		copyMemory(game->world.entities, game->level.entities, sizeof(entity)*MAX_ENTITIES);
	}
	if(keyUpEvent(win, KEY_F2)) {
		game->debug.showUI = !game->debug.showUI;
	}
	
	//glEnable(GL_DEPTH_TEST);
	gfx_update_shaders();
	glClearColor(1, 0.0, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT /*| GL_DEPTH_BUFFER_BIT*/);
	
	//glLoadIdentity();
	
	//glPushMatrix();
	//glScalef(xscale, yscale, 1);
	//glTranslatef(-game->camx, -game->camy, 0);
	
	// 	glBindTexture(GL_TEXTURE_2D, tileTex);
	// 	glBegin(GL_QUADS);
	// 	glColor3f(1, 1, 1); glTexCoord2f(0, 0); glVertex2f(-0.5, -0.5);
	// 	glColor3f(1, 1, 1); glTexCoord2f(1, 0); glVertex2f(0.5, -0.5);
	// 	glColor3f(1, 1, 1); glTexCoord2f(1, 1); glVertex2f(0.5, 0.5);
	// 	glColor3f(1, 1, 1); glTexCoord2f(0, 1); glVertex2f(-0.5, 0.5);
	// 	glEnd();
	
	// 	glEnable(GL_MODELVIEW_MATRIX);
	
	//glColor3f(1, 1, 1);
	// 	{
	// 		int x;
	// 		int y;
	// 		for(y=-2; y<3; ++y) {
	// 			for(x=-4; x<4; ++x) {
	// 				drawTile(tiles[(y+2)*8+(x+4)], x, y);
	// 			}
	// 		}
	// 	}
	//drawTile(15, 0, 0);
	// 	drawTile(0, 1, 0);
	// 	drawTile(2, 0, 1);
	// 	drawTile(3, 1, 1);
	
	// 	{
	// 		glColor3f(1, 0, 0);
	// 		glBegin(GL_QUADS);
	// 		glVertex3f(-.5 + mx*1, -.5 + my*1, 0);
	// 		glVertex3f(.5 + mx*1, -.5 + my*1, 0);
	// 		glVertex3f(.5 + mx*1, .5 + my*1, 0);
	// 		glVertex3f(-.5 + mx*1, .5 + my*1, 0);
	// 		glEnd();
	// 	}

	tile* tiles = game->level.tiles;
	building* buildings = game->level.buildings;
	
	//glEnable(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, game->tileTex);
	//gfx_sh(0);
	{
		int x,y,i;
		for(i=0; i<game->level.buildingCount; ++i) {
			building* b = buildings+i;
			for(y=0; y<b->size.y-0.5f; ++y) for(x=0; x<b->size.x-0.5f; ++x) {
				float spriteOffset = -0.499f;
				vec3 tpos = _vec3(b->pos.x + x, b->pos.y + y + spriteOffset, y);
				
				//glColor3f(1, 1, 1);
				//gfx_sh(game->tileShader);
				if(y==0) {
					// bottom front
					if(x==0) {
						rPushTile(1, tpos);
					} else if(x==b->size.x-1) {
						rPushTile(2, tpos);
					} else {
						if(x==1) {
							rPushTile(6, tpos);
						} else {
							rPushTile(6, tpos);
						}
					}
				} else if(y==b->size.y-1) {
					// roof back
					tpos.y = b->pos.y+(y-1) + spriteOffset;
					tpos.z = 1;
					if(x==0) {
						rPushTile(12, tpos);
					} else if(x==b->size.x-1) {
						rPushTile(13, tpos);
					} else {
						rPushTile(11, tpos);
					}
				} else if(y==1) {
					// upper front and roof front
					tpos.y = b->pos.y + spriteOffset;
					tpos.z = 1;
					if(x==0) {
						rPushTile(4, tpos);
						tpos.z += 0.09f;
						rPushTile(9, tpos);
					} else if(x==b->size.x-1) {
						rPushTile(5, tpos);
						tpos.z += 0.09f;
						rPushTile(10, tpos);
					} else {
						rPushTile(3, tpos);
						tpos.z += 0.09f;
						rPushTile(8, tpos);
					}
				} else {
					// roof middle
					tpos.y = b->pos.y+(y-1) + spriteOffset;
					tpos.z = 1;
					if(x==0) {
						rPushTile(17, tpos);
					} else if(x==b->size.x-1) {
						rPushTile(18, tpos);
					} else {
						rPushTile(16, tpos);
					}
				}
				//gfx_sh(0);
			}
		}
	}
	
	//glColor3f(1, 1, 1);
	//gfx_sh(game->tileShader);
	//gfx_ut("texture0", game->tileTex);
	{
		int x,y,i=0;
		for(y=-h; y<-h+WORLD_SIZE; ++y) for(x=-h; x<-h+WORLD_SIZE; ++x) {
			if(tiles[i].type) {
				if(tiles[i].type==TILE_FENCE) {
					int left = tiles[i-1].type==TILE_FENCE;
					int right = tiles[i+1].type==TILE_FENCE;
					int above = tiles[i+WORLD_SIZE].type==TILE_FENCE;
					int below = tiles[i-WORLD_SIZE].type==TILE_FENCE;
					int fenceHori = left || right;
					int fenceVert = above || below;
					if(!fenceHori && !fenceVert) {
						rPushTile(tiles[i].type-1, _vec3(x,y-0.5f,0));
					} else if(!fenceHori && fenceVert) {
						rPushTile(tiles[i].type+0, _vec3(x,y-0.5f,0));
					} else if(fenceHori && !fenceVert) {
						rPushTile(tiles[i].type-1, _vec3(x,y-0.5f,0));
					} else {
						if(left) {
							rPushTile(tiles[i].type+1, _vec3(x,y-0.5f,0));
						}
						if(right) {
							rPushTile(tiles[i].type+2, _vec3(x,y-0.5f,0));
						}
						if(above) {
							rPushTile(tiles[i].type+3, _vec3(x,y-0.5f,0));
						}
						if(below) {
							rPushTile(tiles[i].type+4, _vec3(x,y-0.5f,0));
						}
					}
				} else {
					rPushTile(tiles[i].type-1, _vec3(x,y-0.5f,0));
				}
			}
			
			++i;
		}
	}
	
	// entities
	entity* entities = game->level.entities;
	if(game->mode) {
		entities = game->world.entities;
	}
	for(int i=0; i<game->level.entityCount; ++i) {
		entity* e = entities+i;
		
		tile* selectedTile = getTile(game->editor.select);
		rPushSprite(sprite(game->charactersTexture, 0, 32*(selectedTile->hasEntity && selectedTile->entityIndex==i ? game->editor.editEntity.sprite : e->sprite), 32, 32), e->pos.xy, 0);
	}
	
	//glDisable(GL_TEXTURE_2D);
	
	/*glColor3f(1, 1, 1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, cabinTex);
	glBegin(GL_QUADS);
	glTexCoord2f(.0f, .0f); glVertex2f(-0.5*4, -0.5);
	glTexCoord2f(.25f, .0f); glVertex2f(0.5*4, -0.5);
	glTexCoord2f(.25f, .25f); glVertex2f(0.5*4, 0.5+3);
	glTexCoord2f(.0f, .25f); glVertex2f(-0.5*4, 0.5+3);
	glEnd();*/
	
	if(game->mode) {
		int moved = 0;
		if(!uiAnyActiveWidget()) {
			// player movement
			vec2 movement = {0};
			if(keyIsDown(win, KEY_UP)) movement.y += 1.0f;
			if(keyIsDown(win, KEY_DOWN)) movement.y -= 1.0f;
			if(keyIsDown(win, KEY_LEFT)) movement.x -= 1.0f;
			if(keyIsDown(win, KEY_RIGHT)) movement.x += 1.0f;
			
			if(movement.x!=0.0f || movement.y!=0.0f) {
				moved = 1;
				movement = normalize2(movement);
				vec2 velocity = mul2(movement, _vec2(3.0f*dt, 3.0f*dt));
				vec2 newPos = add2(game->playerPos, velocity);
				
				// todo: collisions
				//point newTilePos = positionToTileStorageCoords(newPos);
				//tile newTile = game->level.tiles[newTilePos.y*WORLD_SIZE+newTilePos.x];
				//if(!newTile.type || newTile.type==TILE_DOOR) {
					game->playerPos = newPos;
				//}
				
				game->playerFacingDir = movement.x>0.0f;
			}
			
			game->camx = game->playerPos.x;
			game->camy = game->playerPos.y;
			
			// dialog
			if(keyUpEvent(win, 'X')) {
				if(game->dialogs.current) {
					if(game->dialogs.line < game->dialogs.current->count-1) {
						++game->dialogs.line;
						game->dialogs.typer = 0;
					} else {
						game->dialogs.current = 0;
					}
				} else {
					float distToSanta = len2(sub2(_vec2(0,0), game->playerPos));
					if(!game->dialogs.current && distToSanta < 1.0f) {
						game->dialogs.current = &game->dialogs.santaIntro;
						game->dialogs.line = 0;
						game->dialogs.typer = 0;
					}
				}
			}
		}
		
		//gfx_sh(0);
		
		if(moved) {
			game->playerAniFrame += 18.0f*dt;
			if(game->playerAniFrame > 10) game->playerAniFrame -= 9;
		} else {
			game->playerAniFrame = 0;
		}
		
		//glColor3f(1, 1, 1);
		//gfx_sh(tileShader);
		//gfx_ut("texture0", game->tileTex);
		/*glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, game->charactersTexture);
		
		glColor3f(1, 1, 1);
		glPushMatrix();
		glTranslatef(game->playerPos.x, game->playerPos.y, 0.0f);
		glBegin(GL_QUADS);
		glTexCoord2f((game->playerFacingDir?.125f:.0f) + ((int)game->playerAniFrame%8)*.125f, 1-.125f - ((int)game->playerAniFrame/8)*.125f);
		glVertex2f(-0.5, -0.5);
		glTexCoord2f((game->playerFacingDir?.0f:.125f) + ((int)game->playerAniFrame%8)*.125f, 1-.125f - ((int)game->playerAniFrame/8)*.125f);
		glVertex2f(0.5, -0.5);
		glTexCoord2f((game->playerFacingDir?.0f:.125f) + ((int)game->playerAniFrame%8)*.125f, 1-.0f - ((int)game->playerAniFrame/8)*.125f);
		glVertex2f(0.5, 0.5);
		glTexCoord2f((game->playerFacingDir?.125f:.0f) + ((int)game->playerAniFrame%8)*.125f, 1-.0f - ((int)game->playerAniFrame/8)*.125f);
		glVertex2f(-0.5, 0.5);
		glEnd();
		glPopMatrix();*/
		
		//rPushSprite(game->charactersTexture, game->playerPos,
					//_vec2(((int)game->playerAniFrame%8)*.125f,
						  //1-.125f - ((int)game->playerAniFrame/8)*.125f),
		//_vec2(.125f, .125f), game->playerFacingDir);
		rPushSprite(sprite(game->charactersTexture, 32*(int)game->playerAniFrame, 0, 32, 32),
					game->playerPos, game->playerFacingDir);
		
		rPushDebugPoint(_vec2to3(game->playerPos, 0), _vec4(0,1,0,1));
		
		/*					_vec2(game->playerFacingDir?.125f:.0f + ((int)game->playerAniFrame%8)*.125f,
						  1-.125f - ((int)game->playerAniFrame/8)*.125f),
					_vec2(game->playerFacingDir?-.125f:.125f, .125f)*/
		
		//rPushSprite(game->charactersTexture, _vec2(0, 0),
					//_vec2(0.0f, 1.0f-0.125*3), _vec2(0.125f, 0.125f), 0);
		
		for(int y=-h; y<-h+WORLD_SIZE; ++y) for(int x=-h; x<-h+WORLD_SIZE; ++x) {
			
		}
		
		// 3D example
#if 0
		vec3 points[8];
		points[0] = _vec3(-0.5f, -0.5f, 0);
		points[1] = _vec3(0.5f, -0.5f, 0);
		points[2] = _vec3(0.5f, 0.5f, 0);
		points[3] = _vec3(-0.5f, 0.5f, 0);
		points[4] = _vec3(-0.5f, -0.5f, 1);
		points[5] = _vec3(0.5f, -0.5f, 1);
		points[6] = _vec3(0.5f, 0.5f, 1);
		points[7] = _vec3(-0.5f, 0.5f, 1);
		mat4 r = mat4_identity();
		mat4_rotate_z(&r, platform->runtimeInSeconds);
		for(int i=0; i<8; ++i) {
			points[i] = vec4_mul_mat4(_vec3to4(points[i], 0), r).xyz;
			rPushDebugPoint(mul3f(points[i],0.5f), _vec4(1,0,0,1));
		}
#endif
	} else {
		// editor
		
		float yscale = 32.f*4.f * (2.f/win->height) *1;
		float xscale = yscale * ((float)win->height/win->width);
		int h = WORLD_SIZE/2;
		float mx = floorf(clamp(((float)win->mouse.x/(win->width/2)-1)/xscale +h +.5f + game->camx, 0, WORLD_SIZE-1) - h);
		float my = floorf(clamp(((float)win->mouse.y/(win->height/2)-1)*-1/yscale +h +.5f + game->camy, 0, WORLD_SIZE-1) - h);
		if(win->keyboard['W'] & 0x80) game->camy += 0.05f;
		if(win->keyboard['S'] & 0x80) game->camy -= 0.05f;
		if(win->keyboard['D'] & 0x80) game->camx += 0.05f;
		if(win->keyboard['A'] & 0x80) game->camx -= 0.05f;
		
		//point mouseTileCoord = worldCoordsToStorageCoords(_point(mx, my));
		if(platform->rbuttonDownEvent) {
			game->editor.select = _vec3(mx, my, 0);
			
			//game->editor.editEntity = game->level.entities[game->level.tiles[game->editor.select.y*WORLD_SIZE+game->editor.select.x].entityIndex];
			game->editor.editEntity = game->level.entities[getTile(game->editor.select)->entityIndex];
		}
		if(platform->rbuttonIsDown) {
			//game->editor.dragx = mx;
			//game->editor.dragy = my;
			//point drag = worldCoordsToStorageCoords(_point(mx, my));
			game->editor.selectEnd = _vec3(mx, my, 0);
		}
		if(platform->lbuttonDownEvent) {
			game->editor.insert = _vec3(mx, my, 0);
		}
		if(platform->lbuttonIsDown) {
			game->editor.insertEnd = _vec3(mx, my, 0);
		}
		
		// selected tile
		vec3 selectStart = min3(game->editor.select, game->editor.selectEnd);
		vec3 selectEnd = max3(game->editor.select, game->editor.selectEnd);
		vec3 insertStart = min3(game->editor.insert, game->editor.insertEnd);
		vec3 insertEnd = max3(game->editor.insert, game->editor.insertEnd);
		//vec2 selectedPos = storageCoordsToWorldCoords(_vec2(lerp(selectEnd.x, selectStart.x, 0.5f), lerp(selectEnd.y, selectStart.y, 0.5f)));
		tile* selectedTile = getTile(game->editor.select);
		tile* mouseTile = getTile(_vec3(mx, my, 0));
		rPushDebugLineQuad(lerp3(selectEnd, selectStart, 0.5f), 
						   _vec3(selectEnd.x-selectStart.x+1,
								 selectEnd.y-selectStart.y+1,0), ORANGE, 1);
		if(platform->lbuttonIsDown) {
			rPushDebugLineQuad(lerp3(insertEnd, insertStart, 0.5f), 
							   _vec3(insertEnd.x-insertStart.x+1,
									 insertEnd.y-insertStart.y+1,0), PURPLE, 1);
		}
		
		// editor ui
		static int entityTypeTest = ENTITY_ACTOR;
		uiWindowParent("Editor window", 0) {
			uiPushText("Editor");
			uiPushDropdown("Mode", &game->editor.mode, editorModeNames, c_array_size(editorModeNames));
			
			if(uiPushButton("Door")->state->clicked) game->editor.tile = 1;
			if(uiPushButton("Window")->state->clicked) game->editor.tile = 25;
			if(uiPushButton("Fence")->state->clicked) game->editor.tile = 27;
			
			uiPushText(stringFormat("select (%.1f,%.1f), (%.1f,%.1f)",
											   game->editor.select.x, game->editor.select.y,
											   game->editor.selectEnd.x, game->editor.selectEnd.y));
			uiPushText(stringFormat("mouse %.0f,%.0f", mx, my));
			//uiPushSprite(sprite(game->charactersTexture, 0, 128+32, 32, 32));
		}
		
		if(selectedTile->hasEntity) {
			entity* e = &game->level.entities[selectedTile->entityIndex];
			uiWindowParent("Entity window", 0) {
				uiPushText(stringFormat("Tile (%i,%i)\nEntity (%s)", game->editor.select.x, game->editor.select.y, e->name));
				uiPushText("Name:")->spacing = 0;
				ui_widget* nameInput = uiPushInput(stringFormat("Name %s", e->name), e->name, 16);
				nameInput->xSize = uiFractionOfParentSize(1.0f);
				uiPushDropdown("Entity type", &game->editor.editEntity.type, entityTypeNames, 2);	
				//uiPushText("Use MouseR to place route");
				if(uiPushButton("Choose sprite")->state->clicked) {
					game->editor.chooseSprite = !game->editor.chooseSprite;
				}
				if(uiPushButton("Save entity")->state->clicked) {
					*e = game->editor.editEntity;
					saveLevelFile(&game->level);
					game->editor.select = _vec3(0,0,0);
					game->editor.selectEnd = _vec3(0,0,0);
				}
				copyMemory(game->editor.editEntity.name, nameInput->state->text, c_array_size(e->name));
			}
			
			if(game->editor.chooseSprite) {
				uiWindowParent("Sprite select window", 0) {
					uiPushText("Sprites");
					ui_widget* row = uiPushWindow("whatever", 0);
					row->layout = UI_LAYOUT_H;
					//row->flags &= ~UI_FLAG_BACKGROUND;
					row->flags ^= UI_FLAG_BOX;
					uiPushParent(row);
					for(int i=0; i<8; ++i) {
						ui_widget* button = uiPushButton(stringFormat("Sprite %i", i));
						button->flags &= ~UI_FLAG_TEXT;
						button->flags &= ~UI_FLAG_BEVEL;
						button->padding = _vec2(0,0);
						uiPushParent(button);
						uiPushSprite(sprite(game->charactersTexture, 0, 32*i, 32, 32))->spacing = 0;;
						uiPopParent();
						if(button->state->clicked) {
							game->editor.editEntity.sprite = i;
							game->editor.chooseSprite = 0;
						}
					}
					uiPopParent();
				}
			}
		} else {
			game->editor.chooseSprite = 0;
		}
		
		tile* tiles = game->level.tiles;
		building* buildings = game->level.buildings;
		
		if(!uiAnyActiveWidget()) {
			if(game->editor.mode == EDITOR_MODE_TILE) {
				if(win->lbuttonUpEvent) {
					if(game->editor.select.x==game->editor.selectEnd.x &&
					   game->editor.select.y==game->editor.selectEnd.y) {
						
						mouseTile->type = game->editor.tile;
						printf("mouse x %i\n", win->mouse.x);
						saveLevelFile(&game->level);
					}
				}
				// old deletion
				//if(win->rbuttonUpEvent) {
					//selectedTile->type = 0;
					//saveLevelFile(&game->level);
				//}
			}
			
			if(game->editor.mode == EDITOR_MODE_BUILDING) {
				if(win->lbuttonUpEvent) {
					//point dragMin = {imin(game->editor.dragx,mx), imin(game->editor.dragy,my)};
					//point dragMax = {imax(game->editor.dragx,mx), imax(game->editor.dragy,my)};
					if(game->level.buildingCount < MAX_BUILDINGS) {
						int intersection = 0;
						for(int i=0; i<game->level.buildingCount; ++i) {
							building b = buildings[i];
							if(insertStart.x < b.pos.x+b.size.x &&
							   insertEnd.x >= b.pos.x &&
							   insertStart.y < b.pos.y+b.size.y &&
							   insertEnd.y >= b.pos.y) {
								intersection = 1;
								break;
							}
						}
						if(!intersection && insertEnd.x-insertStart.x+1 > 2 && insertEnd.y-insertStart.y+1 > 2) {
							buildings[game->level.buildingCount].pos = insertStart;
							buildings[game->level.buildingCount].size = sub3(add3f(insertEnd, 1), insertStart);
							++game->level.buildingCount;
							printf("added building \n");
							saveLevelFile(&game->level);
						}
					}
				}
				/*if(win->rbuttonUpEvent) {
					for(int i=0; i<game->level.buildingCount; ++i) {
						if(mx>buildings[i].pos.x && mx<buildings[i].pos.x+buildings[i].size.x &&
						   my>buildings[i].pos.y && my<buildings[i].pos.y+buildings[i].size.y) {
							copyMemory(buildings+i, buildings+i+1,
									   (game->level.buildingCount-(i+1))*sizeof(building));
							--game->level.buildingCount;
							saveLevelFile(&game->level);
						}
					}
				}*/
			}
			
			if(game->editor.mode == EDITOR_MODE_ENTITY) {
				if(win->lbuttonUpEvent &&
				   diff(game->editor.insert.x, game->editor.insertEnd.x) < 0.1f &&
				   diff(game->editor.insert.y, game->editor.insertEnd.y) < 0.1f) {
					//if(tileIndex>0 && tileIndex<WORLD_SIZE*WORLD_SIZE) {
						//tile* t = &tiles[tileIndex];
					if(game->level.entityCount<MAX_ENTITIES && !mouseTile->hasEntity) {
						mouseTile->hasEntity = TRUE;
						mouseTile->entityIndex = game->level.entityCount;
						entity e = {0};
						e.pos = _vec3(mx,my,0);
						char* s = stringFormat("Entity %i", game->level.entityCount);
						copyMemory(e.name, s, stringLength(s));
						game->level.entities[game->level.entityCount++] = e;
						uiMessage("Entity added");
						game->editor.select = _vec3(mx,my,0);
						game->editor.selectEnd = _vec3(mx,my,0);
						game->editor.editEntity = game->level.entities[mouseTile->entityIndex];
					}
					//}
				}
			}
			
			//if(game->editor.mode == EDITOR_MODE_SELECT) {
								
			//}
		}
		
		int x,y;
		for(y=-h; y<-h+WORLD_SIZE; ++y) for(x=-h; x<-h+WORLD_SIZE; ++x) {
			//if((my==y && mx==x) || win->lbuttonIsDown && (x>=dragMin.x && x<=dragMax.x && y>=dragMin.y && y<=dragMax.y)) {
				// glColor4f(.6, .8, 1, .5);
				
				//vec4 color = _vec4(1, 1, 1, .5);
				//if(x==dragMin.x || x==dragMax.x || y==dragMin.y || y==dragMax.y) {
					//color = _vec4(1, .5, .5, .5);
				//}
				//rPushDebugQuad(_vec3(x,y,0), _vec3(1,1,0), color);
			//}
			if(/*game->editor.showGrid*/1) {
				vec4 color = _vec4(0,0,0,1);
				//rPushDebugPoint(_vec3(x,y,0), _vec4(1,0,0,1));
				rPushDebugLine(_vec3(x-0.5f, y-0.5f, 0),
							   _vec3(1, 0, 0),
							   color);
				rPushDebugLine(_vec3(x+0.5f, y-0.5f, 0),
							   _vec3(0, 1, 0),
							   color);
				/*rPushDebugLine(_vec3(x+0.5f, y+0.5f, 0),
							   _vec3(x+0.5f, y+0.5f, 0),
							   color);
				rPushDebugLine(_vec3(x-0.5f, y+0.5f, 0),
							   _vec3(x-0.5f, y-0.5f, 0),
							   color);*/
			}
		}
	}
	
	rRender();
	
	/*glDisable(GL_TEXTURE_2D);
	glColor3f(0,1,0);
	glPointSize(4.0f);
	glBegin(GL_POINTS);
	glVertex2f(game->playerPos.x, game->playerPos.y);
	glEnd();*/
	
	//glPopMatrix();
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, game->fontTexture.handle);
	glPushMatrix();
	vec2 uiScale = _vec2(2.0f/(float)win->width * 8.0f*2.0f, 2.0f/(float)win->height * 8.0f*2.0f);
	vec2 uiScreenSize = div2(_vec2(2,2), uiScale);
	//glTranslatef(-1.0f, -1.0f, 0.0f);
	glScalef(uiScale.x, -uiScale.y, 1.0f);
	glTranslatef(-uiScreenSize.x/2.0f, -uiScreenSize.y/2.0f, 0.0f);
	
	uiWrapWidth(64);
	uiFontSize(1);
	ui->font = &game->font;
	//uiText(&game->font, _vec2(1, uiScreenSize.y-2), "Hello World!");
	//uiText(&game->font, _vec2(1, uiScreenSize.y-4), "Player %f %f", game->playerPos.x, game->playerPos.y);
	glColor3f(0,0,0);
	
	stringFormat("time %.1f", game->time);
	uiText(getString(), _vec2(1, ui->screenSize.y-2));
	
	/*char characters[129];
	for(int i=0; i<128; ++i) {
		characters[i] = i+1;
	}
	characters[128] = 0;
	drawText(&game->font, characters, _vec2(1, uiScreenSize.y/2), 2.0f, 30);*/
	
	glColor3f(1,0,0);
	uiFontSize(1.0f);
	char* nsp = "Long ago and far away\nIn labyrinths of coral caves\nA mystic crystal was forged in glass\nFrom magic or some shit like that\nIts powers had been used for good\nAnd all men lived in brotherhood\nUntil the Necromancer came\nUpon his steed of bones and flame";
	//uiText(nsp, _vec2(1, uiScreenSize.y/2));
	
	uiLayout(&game->font);
	uiRender(game, dt);
	
	float distToSanta = len2(sub2(_vec2(0,0), game->playerPos));
	if(!game->dialogs.current && distToSanta < 1.0f) {
		glColor3f(0,0,0);
		uiText("Press X to talk", _vec2(ui->screenSize.x*0.5f-5, ui->screenSize.y-30));
	}

	//game->dialogs.current = &game->dialogs.santaIntro;
	if(game->dialogs.current) {
		gfx_sh(0);
		glDisable(GL_TEXTURE_2D);
		glColor3f(0.1f, 0.2f, 0.5f);
		glPushMatrix();
		glTranslatef(ui->screenSize.x*0.5f, ui->screenSize.y-15, 0);
		glBegin(GL_QUADS);
		glVertex2f(-ui->screenSize.x*0.25f, 0);
		glVertex2f(+ui->screenSize.x*0.25f, 0);
		glVertex2f(+ui->screenSize.x*0.25f, 20);
		glVertex2f(-ui->screenSize.x*0.25f, 20);
		glEnd();
		glPopMatrix();
		
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, game->fontTexture.handle);
		glColor3f(1,1,1);
		dialog_line* line = &game->dialogs.current->lines[game->dialogs.line];
		char* pressx[] = {
			"Press X to continue",
			"Press X to continue.",
			"Press X to continue..",
			"Press X to continue...",
		};
		uiFontSize(1);
		ui_text_layout layout = uiLayoutText("Press X to continue...");
		uiText(pressx[(int)(fmod(game->time, 1.0f)*4.0f)],
			   _vec2(ui->screenSize.x*0.75f-layout.size.x-1, ui->screenSize.y-1.5));
		uiFontSize(1.5f);
		uiText(line->name, _vec2(ui->screenSize.x*0.25f + 1, ui->screenSize.y-2));
		uiFontSize(2);
		uiWrapWidth(ui->screenSize.x*0.5f - 2);
		int typeIndex = imin((int)game->dialogs.typer, stringLength(line->text));
		
		ui_text_layout dialoglayout = uiLayoutText(line->text);
		char tmp = line->text[typeIndex];
		line->text[typeIndex] = 0;
		uiDrawTextLayout(dialoglayout,
						 _vec2(ui->screenSize.x*0.25f + 1, ui->screenSize.y-14));
		line->text[typeIndex] = tmp;
		
		game->dialogs.typer += dt*20.0f;
		typeIndex = imin((int)game->dialogs.typer, stringLength(line->text));
		if(line->text[typeIndex]==' ') {
			game->dialogs.typer += 1;
		}
	}
	
	glPopMatrix();
	
	if(game->debug.showUI) {
		// Memory debug
		glUseProgram(0);
		glDisable(GL_TEXTURE_2D);
		
		glPushMatrix();
		glTranslatef(-0.9f, 0.5f, 0);
		float m = 10.0f / (float)gfx->memorySize;
		glScalef(m, m, 1);
		glColor3f(1, 0, 0);
		/*glBegin(GL_QUADS);
		glVertex2f(0, 0);
		glVertex2f(64*64, 0);
		glVertex2f(64*64, gfx->memorySize/(64*64)*128);
		glVertex2f(0, gfx->memorySize/(64*64)*128);
		glEnd();*/
		int i = 0;
		while(i<gfx->memorySize/64) {
			int x = (((i)%64)*64);
			int y = (((i)/64)*128);
			glBegin(GL_QUADS);
			glVertex2f(x, y);
			glVertex2f(x+64, y);
			glVertex2f(x+64, y+128);
			glVertex2f(x, y+128);
			glEnd();
			++i;
		}
		
		gfx_shader* shader = gfx->shaders;
		while(shader) {
			int offset = ((byte*)shader-gfx->memory);
			int size = align(sizeof(gfx_shader) + shader->path_size + shader->code_size, 64);
			int w = size;
			//glColor3f((float)((uint64_t)shader % 10) / 10.0f, 1, 0);
			glColor3f(0, (float)((uint64_t)shader->code_size % 255) / 255.0f, 0);
			for(int i=offset/64; i<((offset+w)/64); ++i) {
				int x = (((i)%64)*64);
				int y = (((i)/64)*128);
				glBegin(GL_QUADS);
				glVertex2f(x, y);
				glVertex2f(x+64, y);
				glVertex2f(x+64, y+128);
				glVertex2f(x, y+128);
				glEnd();
			}
			
			glBegin(GL_LINES);
			glVertex2f(offset, 0.1f);
			glVertex2f(offset, 0.15f);
			glEnd();
			
			shader = shader->next;
		}
		
		free_block* free = gfx->freeBlocks;
		while(free) {
			int offset = ((byte*)free-gfx->memory);
			int w = free->size;
			glColor3f(0.2f, 0.2f, 0.2f);
			for(int i=offset/64; i<((offset+w)/64); ++i) {
				int x = (((i)%64)*64);
				int y = (((i)/64)*128);
				glBegin(GL_QUADS);
				glVertex2f(x, y);
				glVertex2f(x+64, y);
				glVertex2f(x+64, y+128);
				glVertex2f(x, y+128);
				glEnd();
			}
			
			glBegin(GL_LINES);
			glVertex2f(offset, 0.1f);
			glVertex2f(offset, 0.15f);
			glEnd();
			
			free = free->next;
		}
		
		glPopMatrix();
	}
	
	// clean up
	clearMemoryArena(&ui->transient);
	clearMemoryArena(&ui->widgetTree);
	game->ui.widgets = 0;
	stringClear();
}