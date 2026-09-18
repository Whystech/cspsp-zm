#include "Grenade.h"
#include "Globals.h"

//------------------------------------------------------------------------------------------------
Grenade::Grenade(float x, float y, float px, float py, float angle, float speed, Person *parent, int type) : Bullet(x,y,px,py,angle,speed,0,parent)
{
	mType = TYPE_GRENADE;
	mGrenadeType = type;
	mTimer = gGrenadeConfig.fuseTime;
	mSpinAngle = 0.0f;
}

//------------------------------------------------------------------------------------------------
Grenade::~Grenade()
{
}


//------------------------------------------------------------------------------------------------
void Grenade::Update(float dt)
{
	Bullet::Update(dt);
	mTimer -= dt;
	if (mTimer < 0.0f) {
		mState = BULLET_DEAD;
	}
	mSpinAngle += 0.01f*dt;
}



//------------------------------------------------------------------------------------------------
void Grenade::Render(float x, float y)
{
	float offsetX = (x-SCREEN_WIDTH_2);
	float offsetY = (y-SCREEN_HEIGHT_2);

	mRenderer->RenderQuad(mParentGun->mGroundQuad,mX-offsetX,mY-offsetY,mSpinAngle);
	
}

Rocket::Rocket(float x, float y, float px, float py, float angle, float speed, Person *parent) : Grenade(x,y,px,py,angle,speed,parent,HE)
{
	mType = TYPE_ROCKET;
	mExplosionTime = 0.0f;
}

Rocket::~Rocket()
{
}

void Rocket::Detonate(float x, float y)
{
	mX = x;
	mY = y;
	pX = x;
	pY = y;
	mExplosionTime = 0.0f;
	mState = 1;
}

void Rocket::Update(float dt)
{
	if (mState == 0) {
		Bullet::Update(dt);
		return;
	}

	mExplosionTime += dt;
	int style = mParentGun->mExplosionStyle;
	if (style < 0 || style >= MAX_EXPLOSION_STYLES || gRocketExplosionQuads[style].empty() ||
		mExplosionTime >= mParentGun->mExplosionFrameTime*gRocketExplosionQuads[style].size()) {
		mState = BULLET_DEAD;
	}
}

void Rocket::Render(float x, float y)
{
	float offsetX = x-SCREEN_WIDTH_2;
	float offsetY = y-SCREEN_HEIGHT_2;
	if (mState == 0) {
		int style = mParentGun->mProjectileStyle;
		JQuad* projectileQuad = style >= 0 && style < MAX_PROJECTILE_STYLES ? gRocketProjectileQuads[style] : NULL;
		if (projectileQuad == NULL) projectileQuad = mParentGun->mGroundQuad;
		if (projectileQuad != NULL) mRenderer->RenderQuad(projectileQuad,mX-offsetX,mY-offsetY,mAngle);
		return;
	}

	int style = mParentGun->mExplosionStyle;
	if (style < 0 || style >= MAX_EXPLOSION_STYLES || gRocketExplosionQuads[style].empty()) return;
	int frame = (int)(mExplosionTime/mParentGun->mExplosionFrameTime);
	if (frame >= 0 && frame < (int)gRocketExplosionQuads[style].size()) {
		mRenderer->RenderQuad(gRocketExplosionQuads[style][frame],mX-offsetX,mY-offsetY);
	}
}

