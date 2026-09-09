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
	TRACER_SLUG
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
		char style[16];
		TracerConfig config;
		if (sscanf(line,"%d %15s %d %d %d %d %f %f",&id,style,&config.red,&config.green,&config.blue,&config.alpha,&config.length,&config.width) != 8) continue;
		if (id < 0 || id >= MAX_GUNS || config.length < 0.0f || config.width <= 0.0f) continue;

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
	if (config.style != TRACER_BOLT && config.style != TRACER_PLASMA && config.style != TRACER_COMET && config.style != TRACER_PULSE) {
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
