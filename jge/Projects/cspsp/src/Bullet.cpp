#include "Bullet.h"
#include "Person.h"
#include <stdio.h>
#include <string.h>

JRenderer* Bullet::mRenderer = NULL;

enum TracerStyle {
	TRACER_NONE,
	TRACER_LINE,
	TRACER_LASER,
	TRACER_DOT,
	TRACER_DASHED,
	TRACER_BEAM,
	TRACER_PLASMA,
	TRACER_BOLT,
	TRACER_COMET,
	TRACER_PULSE,
	TRACER_RAIL,
	TRACER_SPARK,
	TRACER_NEEDLE,
	TRACER_TWIN,
	TRACER_ZIGZAG,
	TRACER_FLARE,
	TRACER_STREAK,
	TRACER_SLUG,
	TRACER_BLADE,
	TRACER_GAUSS,
	TRACER_LIGHTNING,
	TRACER_SPIRAL,
	TRACER_WAVE,
	TRACER_CHAIN,
	TRACER_RICOCHET,
	TRACER_PENETRATOR,
	TRACER_CHARGE,
	TRACER_FADING,
	TRACER_EXPANDING,
	TRACER_TAPERED,
	TRACER_GRADIENT,
	TRACER_HEAT,
	TRACER_DISRUPTOR,
	TRACER_PARTICLE,
	TRACER_SMOKE,
	TRACER_IMPACT_RING,
	TRACER_IMPACT_BURST,
	TRACER_AFTERIMAGE,
	TRACER_ANIMATED
};

struct TracerConfig {
	int style;
	int red;
	int green;
	int blue;
	int alpha;
	float length;
	float width;
	bool customized;
};

static TracerConfig gTracerConfigs[MAX_GUNS];

static int ClampColor(int value)
{
	if (value < 0) return 0;
	if (value > 255) return 255;
	return value;
}

void Bullet::LoadTracerConfig(const char* filename)
{
	for (int i=0; i<MAX_GUNS; i++) {
		gTracerConfigs[i].style = TRACER_LINE;
		gTracerConfigs[i].red = 255;
		gTracerConfigs[i].green = 165;
		gTracerConfigs[i].blue = 0;
		gTracerConfigs[i].alpha = 225;
		gTracerConfigs[i].length = 0.0f;
		gTracerConfigs[i].width = 1.0f;
		gTracerConfigs[i].customized = false;
	}

	FILE* file = fopen(filename,"r");
	if (file == NULL) return;

	char line[256];
	while (fgets(line,sizeof(line),file) != NULL) {
		if (line[0] == '#' || line[0] == '\r' || line[0] == '\n') continue;

		int id;
		char style[20];
		TracerConfig config;
		if (sscanf(line,"%d %19s %d %d %d %d %f %f",&id,style,&config.red,&config.green,&config.blue,&config.alpha,&config.length,&config.width) != 8) continue;
		if (id < 0 || id >= MAX_GUNS || config.length < 0.0f || config.length > 5000.0f || config.width <= 0.0f || config.width > 20.0f) continue;

		if (strcmp(style,"none") == 0) config.style = TRACER_NONE;
		else if (strcmp(style,"line") == 0) config.style = TRACER_LINE;
		else if (strcmp(style,"laser") == 0) config.style = TRACER_LASER;
		else if (strcmp(style,"dot") == 0) config.style = TRACER_DOT;
		else if (strcmp(style,"dashed") == 0) config.style = TRACER_DASHED;
		else if (strcmp(style,"beam") == 0) config.style = TRACER_BEAM;
		else if (strcmp(style,"plasma") == 0) config.style = TRACER_PLASMA;
		else if (strcmp(style,"bolt") == 0) config.style = TRACER_BOLT;
		else if (strcmp(style,"comet") == 0) config.style = TRACER_COMET;
		else if (strcmp(style,"pulse") == 0) config.style = TRACER_PULSE;
		else if (strcmp(style,"rail") == 0) config.style = TRACER_RAIL;
		else if (strcmp(style,"spark") == 0) config.style = TRACER_SPARK;
		else if (strcmp(style,"needle") == 0) config.style = TRACER_NEEDLE;
		else if (strcmp(style,"twin") == 0) config.style = TRACER_TWIN;
		else if (strcmp(style,"zigzag") == 0) config.style = TRACER_ZIGZAG;
		else if (strcmp(style,"flare") == 0) config.style = TRACER_FLARE;
		else if (strcmp(style,"streak") == 0) config.style = TRACER_STREAK;
		else if (strcmp(style,"slug") == 0) config.style = TRACER_SLUG;
		else if (strcmp(style,"blade") == 0) config.style = TRACER_BLADE;
		else if (strcmp(style,"gauss") == 0) config.style = TRACER_GAUSS;
		else if (strcmp(style,"lightning") == 0) config.style = TRACER_LIGHTNING;
		else if (strcmp(style,"spiral") == 0) config.style = TRACER_SPIRAL;
		else if (strcmp(style,"wave") == 0) config.style = TRACER_WAVE;
		else if (strcmp(style,"chain") == 0) config.style = TRACER_CHAIN;
		else if (strcmp(style,"ricochet") == 0) config.style = TRACER_RICOCHET;
		else if (strcmp(style,"penetrator") == 0) config.style = TRACER_PENETRATOR;
		else if (strcmp(style,"charge_beam") == 0 || strcmp(style,"charge") == 0) config.style = TRACER_CHARGE;
		else if (strcmp(style,"fading_beam") == 0 || strcmp(style,"fading") == 0) config.style = TRACER_FADING;
		else if (strcmp(style,"expanding_beam") == 0 || strcmp(style,"expanding") == 0) config.style = TRACER_EXPANDING;
		else if (strcmp(style,"tapered_beam") == 0 || strcmp(style,"tapered") == 0) config.style = TRACER_TAPERED;
		else if (strcmp(style,"gradient_beam") == 0 || strcmp(style,"gradient") == 0) config.style = TRACER_GRADIENT;
		else if (strcmp(style,"heat_ray") == 0 || strcmp(style,"heat") == 0) config.style = TRACER_HEAT;
		else if (strcmp(style,"disruptor") == 0) config.style = TRACER_DISRUPTOR;
		else if (strcmp(style,"particle_trail") == 0 || strcmp(style,"particle") == 0) config.style = TRACER_PARTICLE;
		else if (strcmp(style,"smoke_trail") == 0 || strcmp(style,"smoke") == 0) config.style = TRACER_SMOKE;
		else if (strcmp(style,"impact_ring") == 0) config.style = TRACER_IMPACT_RING;
		else if (strcmp(style,"impact_burst") == 0) config.style = TRACER_IMPACT_BURST;
		else if (strcmp(style,"afterimage") == 0) config.style = TRACER_AFTERIMAGE;
		else if (strcmp(style,"animated_texture") == 0 || strcmp(style,"animated") == 0) config.style = TRACER_ANIMATED;
		else continue;

		config.red = ClampColor(config.red);
		config.green = ClampColor(config.green);
		config.blue = ClampColor(config.blue);
		config.alpha = ClampColor(config.alpha);
		config.customized = true;
		gTracerConfigs[id] = config;
	}
	fclose(file);
}

//------------------------------------------------------------------------------------------------
Bullet::Bullet(float x, float y, float px, float py, float angle, float speed, int damage, Person *parent)
{
	mRenderer = JRenderer::GetInstance();
	Reset(x,y,px,py,angle,speed,damage,parent);
}

//------------------------------------------------------------------------------------------------
Bullet::~Bullet()
{
}


//------------------------------------------------------------------------------------------------
void Bullet::Update(float dt)
{
	if (mState != 0) {
		mState++;
		if (mState == BULLET_DEAD) mState = BULLET_DEAD;
		return;
	}
	if (mIsFirstUpdate) {
		mIsFirstUpdate = false;
	}
	else {
		pX = mX;
		pY = mY;
	}
	//use a ray to prevent missing a collision? (nvm fixed)
	mX += cosAngle*mSpeed*dt;//cosf(mAngle)*mSpeed*dt;
	mY += sinAngle*mSpeed*dt;//sinf(mAngle)*mSpeed*dt;

}



//------------------------------------------------------------------------------------------------
void Bullet::Render(float x, float y)
{
	mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
	int gunId = (mParentGun != NULL) ? mParentGun->mId : -1;
	if (gunId < 0 || gunId >= MAX_GUNS) return;
	TracerConfig config = gTracerConfigs[gunId];
	if (config.style == TRACER_NONE) return;

	float offsetX = (x-SCREEN_WIDTH_2);
	float offsetY = (y-SCREEN_HEIGHT_2);
	float length = (config.length > 0.0f) ? config.length : mSpeed*70.0f;
	float tailX = length*cosAngle;
	float tailY = length*sinAngle;
	float endX = (mState == 0) ? mX : mEndX;
	float endY = (mState == 0) ? mY : mEndY;
	int signX = (cosAngle > 0) ? 1:-1;
	int signY = (sinAngle > 0) ? 1:-1;
	int darkRed = config.customized ? config.red*2/5 : 100;
	int darkGreen = config.customized ? config.green*2/5 : 65;
	int darkBlue = config.customized ? config.blue*2/5 : 25;

	if (config.style == TRACER_DOT) {
		float size = config.width < 1.0f ? 1.0f : config.width;
		mRenderer->FillRect(endX-offsetX-size,endY-offsetY-size,size*2.0f,size*2.0f,ARGB(config.alpha,config.red,config.green,config.blue));
		return;
	}
	else if (config.style == TRACER_DASHED) {
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		for (int i=0; i<3; i++) {
			float nearScale = i*0.3f;
			float farScale = nearScale+0.15f;
			float nearX = limit(mX-tailX*nearScale,mStartX,signX)-offsetX;
			float nearY = limit(mY-tailY*nearScale,mStartY,signY)-offsetY;
			float farX = limit(mX-tailX*farScale,mStartX,signX)-offsetX;
			float farY = limit(mY-tailY*farScale,mStartY,signY)-offsetY;
			mRenderer->DrawLine(farX,farY,nearX,nearY,config.width,ARGB(config.alpha,config.red,config.green,config.blue));
		}
	}
	else if (config.style == TRACER_LASER) {
		float startX = limit(mX-tailX,mStartX,signX)-offsetX;
		float startY = limit(mY-tailY,mStartY,signY)-offsetY;
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
		mRenderer->DrawLine(startX,startY,endX-offsetX,endY-offsetY,config.width*3.0f,ARGB(config.alpha/3,darkRed,darkGreen,darkBlue));
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		mRenderer->DrawLine(startX,startY,endX-offsetX,endY-offsetY,config.width,ARGB(config.alpha,config.red,config.green,config.blue));
	}
	else if (config.style == TRACER_BEAM) {
		float startX = limit(mX-tailX,mStartX,signX)-offsetX;
		float startY = limit(mY-tailY,mStartY,signY)-offsetY;
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
		mRenderer->DrawLine(startX,startY,endX-offsetX,endY-offsetY,config.width*5.0f,ARGB(config.alpha/4,config.red,config.green,config.blue));
		mRenderer->DrawLine(startX,startY,endX-offsetX,endY-offsetY,config.width*2.0f,ARGB(config.alpha,config.red,config.green,config.blue));
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		mRenderer->DrawLine(startX,startY,endX-offsetX,endY-offsetY,config.width,ARGB(config.alpha,255,255,255));
	}
	else if (config.style == TRACER_PLASMA) {
		float startX = limit(mX-tailX*0.6f,mStartX,signX)-offsetX;
		float startY = limit(mY-tailY*0.6f,mStartY,signY)-offsetY;
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
		mRenderer->DrawLine(startX,startY,endX-offsetX,endY-offsetY,config.width*6.0f,ARGB(config.alpha/5,darkRed,darkGreen,darkBlue));
		mRenderer->DrawLine(startX,startY,endX-offsetX,endY-offsetY,config.width*2.0f,ARGB(config.alpha,config.red,config.green,config.blue));
		mRenderer->FillRect(endX-offsetX-config.width*2.0f,endY-offsetY-config.width*2.0f,config.width*4.0f,config.width*4.0f,ARGB(config.alpha,config.red,config.green,config.blue));
	}
	else if (config.style == TRACER_BOLT) {
		float startX = limit(mX-tailX,mStartX,signX)-offsetX;
		float startY = limit(mY-tailY,mStartY,signY)-offsetY;
		float tipX = endX-offsetX;
		float tipY = endY-offsetY;
		float headLength = 5.0f+config.width*2.0f;
		float sideX = -sinAngle*headLength*0.5f;
		float sideY = cosAngle*headLength*0.5f;
		float baseX = tipX-cosAngle*headLength;
		float baseY = tipY-sinAngle*headLength;
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		mRenderer->DrawLine(startX,startY,tipX,tipY,config.width,ARGB(config.alpha,config.red,config.green,config.blue));
		mRenderer->DrawLine(tipX,tipY,baseX+sideX,baseY+sideY,config.width,ARGB(config.alpha,config.red,config.green,config.blue));
		mRenderer->DrawLine(tipX,tipY,baseX-sideX,baseY-sideY,config.width,ARGB(config.alpha,config.red,config.green,config.blue));
	}
	else if (config.style == TRACER_COMET) {
		float startX = limit(mX-tailX,mStartX,signX)-offsetX;
		float startY = limit(mY-tailY,mStartY,signY)-offsetY;
		float headSize = config.width*2.0f;
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
		mRenderer->DrawLine(startX,startY,endX-offsetX,endY-offsetY,config.width*0.5f,ARGB(config.alpha/3,darkRed,darkGreen,darkBlue));
		mRenderer->DrawLine(limit(mX-tailX*0.45f,mStartX,signX)-offsetX,limit(mY-tailY*0.45f,mStartY,signY)-offsetY,endX-offsetX,endY-offsetY,config.width*1.5f,ARGB(config.alpha,config.red,config.green,config.blue));
		mRenderer->FillRect(endX-offsetX-headSize,endY-offsetY-headSize,headSize*2.0f,headSize*2.0f,ARGB(config.alpha,config.red,config.green,config.blue));
	}
	else if (config.style == TRACER_PULSE) {
		float size = config.width < 1.0f ? 1.0f : config.width;
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
		for (int i=0; i<5; i++) {
			float scale = i*0.2f;
			float pulseX = limit(mX-tailX*scale,mStartX,signX)-offsetX;
			float pulseY = limit(mY-tailY*scale,mStartY,signY)-offsetY;
			int pulseAlpha = config.alpha*(5-i)/5;
			mRenderer->FillRect(pulseX-size,pulseY-size,size*2.0f,size*2.0f,ARGB(pulseAlpha,config.red,config.green,config.blue));
		}
	}
	else if (config.style == TRACER_RAIL) {
		float startX = limit(mX-tailX,mStartX,signX)-offsetX;
		float startY = limit(mY-tailY,mStartY,signY)-offsetY;
		float sideX = -sinAngle*config.width*2.0f;
		float sideY = cosAngle*config.width*2.0f;
		float tipX = endX-offsetX;
		float tipY = endY-offsetY;
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
		mRenderer->DrawLine(startX+sideX,startY+sideY,tipX+sideX,tipY+sideY,config.width,ARGB(config.alpha,config.red,config.green,config.blue));
		mRenderer->DrawLine(startX-sideX,startY-sideY,tipX-sideX,tipY-sideY,config.width,ARGB(config.alpha,config.red,config.green,config.blue));
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		mRenderer->DrawLine(startX,startY,tipX,tipY,config.width*0.5f,ARGB(config.alpha,255,255,255));
	}
	else if (config.style == TRACER_SPARK) {
		float tipX = endX-offsetX;
		float tipY = endY-offsetY;
		float radius = 3.0f+config.width*2.0f;
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
		mRenderer->DrawLine(limit(mX-tailX*0.25f,mStartX,signX)-offsetX,limit(mY-tailY*0.25f,mStartY,signY)-offsetY,tipX,tipY,config.width,ARGB(config.alpha,config.red,config.green,config.blue));
		mRenderer->DrawLine(tipX-radius,tipY,tipX+radius,tipY,config.width*0.6f,ARGB(config.alpha,config.red,config.green,config.blue));
		mRenderer->DrawLine(tipX,tipY-radius,tipX,tipY+radius,config.width*0.6f,ARGB(config.alpha,config.red,config.green,config.blue));
	}
	else if (config.style == TRACER_NEEDLE) {
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		mRenderer->DrawLine(limit(mX-tailX*0.35f,mStartX,signX)-offsetX,limit(mY-tailY*0.35f,mStartY,signY)-offsetY,endX-offsetX,endY-offsetY,config.width*0.5f,ARGB(config.alpha,config.red,config.green,config.blue));
	}
	else if (config.style == TRACER_TWIN) {
		float startX = limit(mX-tailX,mStartX,signX)-offsetX;
		float startY = limit(mY-tailY,mStartY,signY)-offsetY;
		float sideX = -sinAngle*config.width*1.5f;
		float sideY = cosAngle*config.width*1.5f;
		float tipX = endX-offsetX;
		float tipY = endY-offsetY;
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		mRenderer->DrawLine(startX+sideX,startY+sideY,tipX+sideX,tipY+sideY,config.width,ARGB(config.alpha,config.red,config.green,config.blue));
		mRenderer->DrawLine(startX-sideX,startY-sideY,tipX-sideX,tipY-sideY,config.width,ARGB(config.alpha,config.red,config.green,config.blue));
	}
	else if (config.style == TRACER_ZIGZAG) {
		float startX = limit(mX-tailX,mStartX,signX)-offsetX;
		float startY = limit(mY-tailY,mStartY,signY)-offsetY;
		float tipX = endX-offsetX;
		float tipY = endY-offsetY;
		float previousX = startX;
		float previousY = startY;
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
		for (int i=1; i<=5; i++) {
			float scale = i/5.0f;
			float side = (i == 5) ? 0.0f : ((i%2 == 0) ? -config.width*3.0f : config.width*3.0f);
			float nextX = startX+(tipX-startX)*scale-sinAngle*side;
			float nextY = startY+(tipY-startY)*scale+cosAngle*side;
			mRenderer->DrawLine(previousX,previousY,nextX,nextY,config.width,ARGB(config.alpha,config.red,config.green,config.blue));
			previousX = nextX;
			previousY = nextY;
		}
	}
	else if (config.style == TRACER_FLARE) {
		float tipX = endX-offsetX;
		float tipY = endY-offsetY;
		float radius = 4.0f+config.width*3.0f;
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
		mRenderer->DrawLine(limit(mX-tailX*0.4f,mStartX,signX)-offsetX,limit(mY-tailY*0.4f,mStartY,signY)-offsetY,tipX,tipY,config.width*2.0f,ARGB(config.alpha/2,config.red,config.green,config.blue));
		mRenderer->DrawLine(tipX-radius,tipY,tipX+radius,tipY,config.width,ARGB(config.alpha,config.red,config.green,config.blue));
		mRenderer->DrawLine(tipX,tipY-radius,tipX,tipY+radius,config.width,ARGB(config.alpha,config.red,config.green,config.blue));
		mRenderer->DrawLine(tipX-radius*0.7f,tipY-radius*0.7f,tipX+radius*0.7f,tipY+radius*0.7f,config.width,ARGB(config.alpha,config.red,config.green,config.blue));
		mRenderer->DrawLine(tipX-radius*0.7f,tipY+radius*0.7f,tipX+radius*0.7f,tipY-radius*0.7f,config.width,ARGB(config.alpha,config.red,config.green,config.blue));
	}
	else if (config.style == TRACER_STREAK) {
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
		mRenderer->DrawLine(limit(mX-tailX,mStartX,signX)-offsetX,limit(mY-tailY,mStartY,signY)-offsetY,endX-offsetX,endY-offsetY,config.width*0.5f,ARGB(config.alpha/3,config.red,config.green,config.blue));
		mRenderer->DrawLine(limit(mX-tailX*0.35f,mStartX,signX)-offsetX,limit(mY-tailY*0.35f,mStartY,signY)-offsetY,endX-offsetX,endY-offsetY,config.width*1.5f,ARGB(config.alpha,config.red,config.green,config.blue));
	}
	else if (config.style == TRACER_SLUG) {
		float startX = limit(mX-tailX*0.18f,mStartX,signX)-offsetX;
		float startY = limit(mY-tailY*0.18f,mStartY,signY)-offsetY;
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		mRenderer->DrawLine(startX,startY,endX-offsetX,endY-offsetY,config.width*3.0f,ARGB(config.alpha,darkRed,darkGreen,darkBlue));
		mRenderer->DrawLine(startX,startY,endX-offsetX,endY-offsetY,config.width,ARGB(config.alpha,config.red,config.green,config.blue));
	}
	else if (config.style == TRACER_BLADE) {
		float tipX = endX-offsetX;
		float tipY = endY-offsetY;
		float radius = 5.0f+config.width*3.0f;
		float spin = (mX+mY)*0.12f;
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
		mRenderer->DrawLine(limit(mX-tailX*0.3f,mStartX,signX)-offsetX,limit(mY-tailY*0.3f,mStartY,signY)-offsetY,tipX,tipY,config.width*2.0f,ARGB(config.alpha/3,darkRed,darkGreen,darkBlue));
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		for (int i=0; i<4; i++) {
			float angle = spin+i*1.5707963f;
			float innerX = tipX+cosf(angle)*radius*0.25f;
			float innerY = tipY+sinf(angle)*radius*0.25f;
			float outerX = tipX+cosf(angle)*radius;
			float outerY = tipY+sinf(angle)*radius;
			float edgeX = outerX+cosf(angle-1.1f)*radius*0.65f;
			float edgeY = outerY+sinf(angle-1.1f)*radius*0.65f;
			mRenderer->DrawLine(innerX,innerY,outerX,outerY,config.width*2.0f,ARGB(config.alpha,darkRed,darkGreen,darkBlue));
			mRenderer->DrawLine(outerX,outerY,edgeX,edgeY,config.width*2.0f,ARGB(config.alpha,darkRed,darkGreen,darkBlue));
			mRenderer->DrawLine(innerX,innerY,outerX,outerY,config.width,ARGB(config.alpha,config.red,config.green,config.blue));
			mRenderer->DrawLine(outerX,outerY,edgeX,edgeY,config.width,ARGB(config.alpha,config.red,config.green,config.blue));
		}
		mRenderer->FillRect(tipX-config.width,tipY-config.width,config.width*2.0f,config.width*2.0f,ARGB(config.alpha,config.red,config.green,config.blue));
	}
	else if (config.style == TRACER_GAUSS || config.style == TRACER_CHARGE) {
		float startX = limit(mX-tailX,mStartX,signX)-offsetX;
		float startY = limit(mY-tailY,mStartY,signY)-offsetY;
		float glow = config.style == TRACER_CHARGE ? 8.0f : 6.0f;
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
		mRenderer->DrawLine(startX,startY,endX-offsetX,endY-offsetY,config.width*glow,ARGB(config.alpha/5,darkRed,darkGreen,darkBlue));
		mRenderer->DrawLine(startX,startY,endX-offsetX,endY-offsetY,config.width*2.0f,ARGB(config.alpha,config.red,config.green,config.blue));
		mRenderer->DrawLine(startX,startY,endX-offsetX,endY-offsetY,config.width*0.6f,ARGB(config.alpha,255,255,255));
	}
	else if (config.style == TRACER_LIGHTNING || config.style == TRACER_WAVE || config.style == TRACER_HEAT) {
		float startX = limit(mX-tailX,mStartX,signX)-offsetX;
		float startY = limit(mY-tailY,mStartY,signY)-offsetY;
		float tipX = endX-offsetX;
		float tipY = endY-offsetY;
		float previousX = startX;
		float previousY = startY;
		float sideScale = config.style == TRACER_HEAT ? config.width*4.0f : config.width*3.0f;
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
		for (int i=1; i<=8; i++) {
			float scale = i/8.0f;
			float side = (i == 8) ? 0.0f : sinf((i+(mState*2))*1.7f)*sideScale;
			if (config.style == TRACER_LIGHTNING) side += ((i*7+mState*3)%5-2)*config.width;
			float nextX = startX+(tipX-startX)*scale-sinAngle*side;
			float nextY = startY+(tipY-startY)*scale+cosAngle*side;
			mRenderer->DrawLine(previousX,previousY,nextX,nextY,config.width,ARGB(config.alpha,config.red,config.green,config.blue));
			previousX = nextX;
			previousY = nextY;
		}
	}
	else if (config.style == TRACER_SPIRAL) {
		float startX = limit(mX-tailX,mStartX,signX)-offsetX;
		float startY = limit(mY-tailY,mStartY,signY)-offsetY;
		float tipX = endX-offsetX;
		float tipY = endY-offsetY;
		for (int strand=0; strand<2; strand++) {
			float previousX = startX;
			float previousY = startY;
			for (int i=1; i<=8; i++) {
				float scale = i/8.0f;
				float side = sinf(scale*12.56637f+strand*3.14159f+mState*0.8f)*config.width*3.0f;
				float nextX = startX+(tipX-startX)*scale-sinAngle*side;
				float nextY = startY+(tipY-startY)*scale+cosAngle*side;
				mRenderer->DrawLine(previousX,previousY,nextX,nextY,config.width*0.7f,ARGB(config.alpha,config.red,config.green,config.blue));
				previousX = nextX;
				previousY = nextY;
			}
		}
	}
	else if (config.style == TRACER_CHAIN || config.style == TRACER_RICOCHET || config.style == TRACER_PENETRATOR) {
		float startX = limit(mX-tailX,mStartX,signX)-offsetX;
		float startY = limit(mY-tailY,mStartY,signY)-offsetY;
		float tipX = endX-offsetX;
		float tipY = endY-offsetY;
		float previousX = startX;
		float previousY = startY;
		int segments = config.style == TRACER_CHAIN ? 6 : 3;
		for (int i=1; i<=segments; i++) {
			float scale = i/(float)segments;
			float side = (i == segments) ? 0.0f : ((i%2 == 0) ? -1.0f:1.0f)*config.width*(config.style == TRACER_RICOCHET ? 8.0f:2.0f);
			float nextX = startX+(tipX-startX)*scale-sinAngle*side;
			float nextY = startY+(tipY-startY)*scale+cosAngle*side;
			mRenderer->DrawLine(previousX,previousY,nextX,nextY,config.width,ARGB(config.alpha,config.red,config.green,config.blue));
			if (config.style == TRACER_CHAIN && i < segments) mRenderer->FillRect(nextX-config.width,nextY-config.width,config.width*2.0f,config.width*2.0f,ARGB(config.alpha,config.red,config.green,config.blue));
			previousX = nextX;
			previousY = nextY;
		}
		if (config.style == TRACER_PENETRATOR) mRenderer->DrawLine(tipX,tipY,tipX+cosAngle*12.0f,tipY+sinAngle*12.0f,config.width*0.7f,ARGB(config.alpha/2,config.red,config.green,config.blue));
	}
	else if (config.style == TRACER_FADING || config.style == TRACER_EXPANDING || config.style == TRACER_TAPERED || config.style == TRACER_GRADIENT) {
		float startX = limit(mX-tailX,mStartX,signX)-offsetX;
		float startY = limit(mY-tailY,mStartY,signY)-offsetY;
		float tipX = endX-offsetX;
		float tipY = endY-offsetY;
		int stateAlpha = config.alpha*(BULLET_DEAD-mState)/BULLET_DEAD;
		for (int i=0; i<6; i++) {
			float from = i/6.0f;
			float to = (i+1)/6.0f;
			float segmentWidth = config.width;
			int segmentAlpha = config.alpha;
			int red = config.red;
			int green = config.green;
			int blue = config.blue;
			if (config.style == TRACER_FADING) segmentAlpha = stateAlpha;
			else if (config.style == TRACER_EXPANDING) segmentWidth *= 1.0f+mState*0.6f;
			else if (config.style == TRACER_TAPERED) segmentWidth *= 2.0f-from*1.7f;
			else {
				red = config.red+(255-config.red)*i/6;
				green = config.green+(255-config.green)*i/6;
				blue = config.blue+(255-config.blue)*i/6;
			}
			mRenderer->DrawLine(startX+(tipX-startX)*from,startY+(tipY-startY)*from,startX+(tipX-startX)*to,startY+(tipY-startY)*to,segmentWidth,ARGB(segmentAlpha,red,green,blue));
		}
	}
	else if (config.style == TRACER_DISRUPTOR || config.style == TRACER_ANIMATED) {
		float startX = limit(mX-tailX,mStartX,signX)-offsetX;
		float startY = limit(mY-tailY,mStartY,signY)-offsetY;
		float tipX = endX-offsetX;
		float tipY = endY-offsetY;
		for (int i=0; i<8; i++) {
			if (config.style == TRACER_DISRUPTOR && (i+mState)%3 == 1) continue;
			if (config.style == TRACER_ANIMATED && (i+mState)%2 == 1) continue;
			float from = i/8.0f;
			float to = (i+0.7f)/8.0f;
			mRenderer->DrawLine(startX+(tipX-startX)*from,startY+(tipY-startY)*from,startX+(tipX-startX)*to,startY+(tipY-startY)*to,config.width,ARGB(config.alpha,config.red,config.green,config.blue));
		}
	}
	else if (config.style == TRACER_PARTICLE || config.style == TRACER_SMOKE) {
		for (int i=0; i<8; i++) {
			float scale = i/7.0f;
			float particleX = endX-tailX*scale-offsetX;
			float particleY = endY-tailY*scale-offsetY;
			float size = config.width*(config.style == TRACER_SMOKE ? 2.0f+scale*2.0f:1.0f);
			int particleAlpha = config.alpha*(8-i)/8;
			mRenderer->FillRect(particleX-size,particleY-size,size*2.0f,size*2.0f,ARGB(particleAlpha,config.red,config.green,config.blue));
		}
	}
	else if (config.style == TRACER_IMPACT_RING || config.style == TRACER_IMPACT_BURST) {
		float tipX = endX-offsetX;
		float tipY = endY-offsetY;
		float radius = (4.0f+mState*2.0f)*config.width;
		int impactAlpha = config.alpha*(BULLET_DEAD-mState)/BULLET_DEAD;
		for (int i=0; i<8; i++) {
			float angle1 = i*0.785398f;
			float angle2 = (i+1)*0.785398f;
			if (config.style == TRACER_IMPACT_RING) mRenderer->DrawLine(tipX+cosf(angle1)*radius,tipY+sinf(angle1)*radius,tipX+cosf(angle2)*radius,tipY+sinf(angle2)*radius,config.width,ARGB(impactAlpha,config.red,config.green,config.blue));
			else mRenderer->DrawLine(tipX+cosf(angle1)*radius*0.25f,tipY+sinf(angle1)*radius*0.25f,tipX+cosf(angle1)*radius,tipY+sinf(angle1)*radius,config.width,ARGB(impactAlpha,config.red,config.green,config.blue));
		}
	}
	else if (config.style == TRACER_AFTERIMAGE) {
		float startX = limit(mX-tailX,mStartX,signX)-offsetX;
		float startY = limit(mY-tailY,mStartY,signY)-offsetY;
		for (int i=0; i<4; i++) {
			float side = (i-1.5f)*config.width*2.0f;
			mRenderer->DrawLine(startX-sinAngle*side,startY+cosAngle*side,endX-offsetX-sinAngle*side,endY-offsetY+cosAngle*side,config.width,ARGB(config.alpha*(4-i)/4,config.red,config.green,config.blue));
		}
	}
	else {
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE);
		mRenderer->DrawLine(limit(mX-tailX,mStartX,signX)-offsetX,limit(mY-tailY,mStartY,signY)-offsetY,endX-offsetX,endY-offsetY,config.width*0.7f,ARGB(config.alpha/3,darkRed,darkGreen,darkBlue));
		mRenderer->DrawLine(limit(mX-tailX*0.75f,mStartX,signX)-offsetX,limit(mY-tailY*0.75f,mStartY,signY)-offsetY,endX-offsetX,endY-offsetY,config.width*0.8f,ARGB(config.alpha*5/9,darkRed,darkGreen,darkBlue));
		mRenderer->DrawLine(limit(mX-tailX*0.25f,mStartX,signX)-offsetX,limit(mY-tailY*0.25f,mStartY,signY)-offsetY,endX-offsetX,endY-offsetY,config.width*0.9f,ARGB(config.alpha,darkRed,darkGreen,darkBlue));
		mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
		mRenderer->DrawLine(limit(mX-tailX*0.75f,mStartX,signX)-offsetX,limit(mY-tailY*0.75f,mStartY,signY)-offsetY,endX-offsetX,endY-offsetY,config.width*0.7f,ARGB(config.alpha/3,config.red,config.green,config.blue));
		mRenderer->DrawLine(limit(mX-tailX*0.5f,mStartX,signX)-offsetX,limit(mY-tailY*0.5f,mStartY,signY)-offsetY,endX-offsetX,endY-offsetY,config.width*0.8f,ARGB(config.alpha*5/9,config.red,config.green,config.blue));
		mRenderer->DrawLine(limit(mX-tailX*0.25f,mStartX,signX)-offsetX,limit(mY-tailY*0.25f,mStartY,signY)-offsetY,endX-offsetX,endY-offsetY,config.width*0.9f,ARGB(config.alpha,config.red,config.green,config.blue));
	}
	if (config.style != TRACER_BOLT && config.style != TRACER_PLASMA && config.style != TRACER_COMET && config.style != TRACER_PULSE && config.style != TRACER_BLADE && config.style != TRACER_PARTICLE && config.style != TRACER_SMOKE && config.style != TRACER_IMPACT_RING && config.style != TRACER_IMPACT_BURST) {
		int endpointAlpha = config.customized ? config.alpha : 255;
		mRenderer->FillRect(endX-offsetX,endY-offsetY-1,1,1,ARGB(endpointAlpha,config.red,config.green,config.blue));
	}
	mRenderer->SetTexBlend(BLEND_SRC_ALPHA, BLEND_ONE_MINUS_SRC_ALPHA);
}


//------------------------------------------------------------------------------------------------
void Bullet::SetAngle(float angle)
{
	mAngle = angle;
	cosAngle = cosf(mAngle);
	sinAngle = sinf(mAngle);
}
//------------------------------------------------------------------------------------------------
void Bullet::Reset(float x, float y, float px, float py, float angle, float speed, int damage, Person *parent)
{
	mState = 0;
	mX = x;
	mY = y;
	pX = px;
	pY = py;
	mStartX = px;
	mStartY = py;
	mEndX = px;
	mEndY = py;
	mAngle = angle;
	mSpeed = speed;
	mDamage = damage;
	mParent = parent;
	mParentGun = parent->mGuns[parent->mGunIndex]->mGun;
	mId = -1;
	mIsFirstUpdate = true;
	mType = TYPE_BULLET;

	cosAngle = cosf(mAngle);
	sinAngle = sinf(mAngle);
}
