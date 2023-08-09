
#include "../math.c"

typedef enum {
	R_JOB_SPRITE,
	R_JOB_DEBUG_POINT,
	R_JOB_DEBUG_LINE,
	R_JOB_DEBUG_QUAD,
	R_JOB_DEBUG_LINE_QUAD,
} r_job_type;
typedef struct {
	r_job_type type;
	//r_texture texture;
	r_sprite sprite;
	vec3 pos;
	vec3 size;
	//vec2 uv;
	//vec2 uv2;
	vec4 color;
	int flip; // used as thick for lines
} r_job;

//void rPushSprite(int texture, vec2 pos, vec2 spritePos, vec2 spriteSize, int flip);
//void rRender();

#ifndef RENDERER_HEADER

#define WIN32_LEAN_AND_MEAN
#include <WINDOWS.H>
#include <gl/GL.H>

#define MAX_JOBS (1024*10)
r_job jobs[MAX_JOBS];
static int numJobs = 0;

inline void rPushJob(r_job job) {
	if(numJobs < MAX_JOBS) {
		jobs[numJobs++] = job;
	}
}

void rPushSprite(r_sprite sprite, vec2 pos, int flip) {
	r_job job = {0};
	job.type = R_JOB_SPRITE;
	//job.texture = sprite.texture;
	job.pos = _vec3(pos.x, pos.y, 0);
	job.sprite = sprite;
	//job.uv = spritePos;
	//job.uv2 = add2(spritePos, spriteSize);
	job.flip = flip;
	rPushJob(job);
}

void rPushGroundTile(int index, float x, float y) {
	/*r_job job = {0};
	job.type = R_JOB_GROUND_TILE;
	job.texture = texture;
	job.pos = pos;
	job.uv = spritePos;
	job.uv2 = add2(spritePos, spriteSize);
	job.flip = flip;
	rPushJob(job);*/
}

void rPushTile(int index, vec3 pos) {
	float ts = 0.125;
	r_job job = {0};
	job.type = R_JOB_SPRITE;
	job.sprite.texture = game->tileTex;
	job.pos = pos;
	job.sprite.uv = _vec2(0+((float)(index%8)*ts), 1-0.125f-((float)(index/8)*ts));
	job.sprite.uv2 = _vec2(ts+((float)(index%8)*ts), 1-((float)(index/8)*ts));
	rPushJob(job);
}

void rPushDebugPoint(vec3 pos, vec4 color) {
	r_job job = {0};
	job.type = R_JOB_DEBUG_POINT;
	job.pos = pos;
	job.color = color;
	rPushJob(job);
}

void rPushDebugLine(vec3 pos, vec3 size, vec4 color) {
	r_job job = {0};
	job.type = R_JOB_DEBUG_LINE;
	job.pos = pos;
	job.size = size;
	job.color = color;
	rPushJob(job);
}

void rPushDebugQuad(vec3 pos, vec3 size, vec4 color) {
	r_job job = {0};
	job.type = R_JOB_DEBUG_QUAD;
	job.pos = pos;
	job.size = size;
	job.color = color;
	rPushJob(job);
}

void rPushDebugLineQuad(vec3 pos, vec3 size, vec4 color, b32 thick) {
	r_job job = {0};
	job.type = R_JOB_DEBUG_LINE_QUAD;
	job.pos = pos;
	job.size = size;
	job.color = color;
	job.flip = thick;
	rPushJob(job);
}

int jobSort(const void* j1, const void* j2) {
	float a = ((r_job*)j1)->pos.y - ((r_job*)j1)->pos.z;
	float b = ((r_job*)j2)->pos.y - ((r_job*)j2)->pos.z;
	int typeCmp = ((r_job*)j1)->type - ((r_job*)j2)->type;
	if(!typeCmp) {
		return (a<b) - (a>b);
	} else {
		return typeCmp;
	}
}

r_sprite sprite(r_texture texture, float x, float y, float w, float h) {
	r_sprite result;
	result.texture = texture;
	result.uv = _vec2(x/texture.width, y/texture.height);
	result.uv2 = add2(result.uv, _vec2(w/texture.width, h/texture.height));
	result.pixelSize = _vec2(w,h);
	return result;
}

void rRender() {
	qsort(jobs, numJobs, sizeof(r_job), jobSort); // todo: replace this
	
	float yscale = 32.f*4.f * (2.f/platform->height) *1;
	float xscale = yscale * ((float)platform->height/platform->width);
	int h = WORLD_SIZE/2;
	glPushMatrix();
	glScalef(xscale, yscale, 1);
	glTranslatef(-game->camx, -game->camy, 0);
	
	// snow
	gfx_sh(game->snowShader);
	gfx_uf("time", platform->runtimeInSeconds);
	gfx_uf2("camera", game->camx, game->camy);
	gfx_uf2("scale", xscale*2, yscale);
	{
		int x,y,i=0;
		for(y=-h; y<-h+WORLD_SIZE; ++y) for(x=-h; x<-h+WORLD_SIZE; ++x) {
			gfx_sh(game->snowShader);
			//drawTile(0, x, y);
			// unbelievably inefficient, todo: sucs
			gfx_uf2("world_position", x, y);
			float z = 0.9f;
			glBegin(GL_QUADS);
			glVertex3f(-0.5, -0.5, z);
			glVertex3f(0.5,  -0.5, z);
			glVertex3f(0.5,  0.5, z);
			glVertex3f(-0.5, 0.5, z);
			glEnd();
			++i;
		}
	}
	
	// world sprites
	gfx_sh(game->tileShader);
	gfx_ut("texture0", game->tileTex.handle);
	glEnable(GL_TEXTURE_2D);
	for(int i=0; i<numJobs; ++i) {
		r_job job = jobs[i];
		if(job.type == R_JOB_SPRITE) {
			job.pos.y += job.pos.z;
			
			glBindTexture(GL_TEXTURE_2D, job.sprite.texture.handle);
			glColor3f(1, 1, 1);
			glPushMatrix();
			glTranslatef(job.pos.x, job.pos.y, 0.0f);
			glBegin(GL_QUADS);
			if(job.flip) {
				glTexCoord2f(job.sprite.uv2.x, job.sprite.uv.y);  glVertex2f(-0.5f, 0.0f);
				glTexCoord2f(job.sprite.uv.x, job.sprite.uv.y);   glVertex2f(0.5f, 0.0f);
				glTexCoord2f(job.sprite.uv.x, job.sprite.uv2.y);  glVertex2f(0.5f, 1.0f);
				glTexCoord2f(job.sprite.uv2.x, job.sprite.uv2.y); glVertex2f(-0.5f, 1.0f);
			} else {
				glTexCoord2f(job.sprite.uv.x, job.sprite.uv.y);  glVertex2f(-0.5f, 0.0f);
				glTexCoord2f(job.sprite.uv2.x, job.sprite.uv.y);   glVertex2f(0.5f, 0.0f);
				glTexCoord2f(job.sprite.uv2.x, job.sprite.uv2.y);  glVertex2f(0.5f, 1.0f);
				glTexCoord2f(job.sprite.uv.x, job.sprite.uv2.y); glVertex2f(-0.5f, 1.0f);
			}
			glEnd();
			glPopMatrix();
			glDisable(GL_TEXTURE_2D);
		}
	}
	
	gfx_sh(0);
	glDisable(GL_TEXTURE_2D);
	for(int i=0; i<numJobs; ++i) {
		r_job job = jobs[i];
		glPointSize(4.0f);
		if(job.type == R_JOB_DEBUG_POINT) {
			job.pos.y += job.pos.z;
			
			glColor4f(job.color.r, job.color.g, job.color.b, job.color.a);
			glBegin(GL_POINTS);
			glVertex2f(job.pos.x, job.pos.y);
			glEnd();
		}
		if(job.type == R_JOB_DEBUG_LINE) {
			job.pos.y += job.pos.z;
			if(job.flip) glLineWidth(4.0f);
			else glLineWidth(1.0f);
			glColor4f(job.color.r, job.color.g, job.color.b, job.color.a);
			glBegin(GL_LINES);
			glVertex2f(job.pos.x, job.pos.y);
			glVertex2f(job.pos.x+job.size.x, job.pos.y+job.size.y);
			glEnd();
		}
		if(job.type == R_JOB_DEBUG_QUAD) {
			job.pos.y += job.pos.z;
			
			glColor4f(job.color.r, job.color.g, job.color.b, job.color.a);
			glBegin(GL_QUADS);
			glVertex2f(job.pos.x-job.size.x*0.5f, job.pos.y-job.size.y*0.5f);
			glVertex2f(job.pos.x+job.size.x*0.5f, job.pos.y-job.size.y*0.5f);
			glVertex2f(job.pos.x+job.size.x*0.5f, job.pos.y+job.size.y*0.5f);
			glVertex2f(job.pos.x-job.size.x*0.5f, job.pos.y+job.size.y*0.5f);
			glEnd();
		}
		if(job.type == R_JOB_DEBUG_LINE_QUAD) {
			job.pos.y += job.pos.z;
			if(job.flip) glLineWidth(4.0f);
			else glLineWidth(1.0f);
			glColor4f(job.color.r, job.color.g, job.color.b, job.color.a);
			glBegin(GL_LINE_STRIP);
			glVertex2f(job.pos.x-job.size.x*0.5f, job.pos.y-job.size.y*0.5f);
			glVertex2f(job.pos.x+job.size.x*0.5f, job.pos.y-job.size.y*0.5f);
			glVertex2f(job.pos.x+job.size.x*0.5f, job.pos.y+job.size.y*0.5f);
			glVertex2f(job.pos.x-job.size.x*0.5f, job.pos.y+job.size.y*0.5f);
			glVertex2f(job.pos.x-job.size.x*0.5f, job.pos.y-job.size.y*0.5f);
			glEnd();
		}
	}
	
	glPopMatrix();
	
	numJobs = 0;
}

#endif