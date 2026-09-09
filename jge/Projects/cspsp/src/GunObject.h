#ifndef _GUNOBJECT_H_
#define _GUNOBJECT_H_

#include "JGE.h"
#include "JRenderer.h"
#include "JSoundSystem.h"
#include <vector>

#define MAX_GUNS 128
#define GUNS_PER_ATLAS 64
#define FIREMODE_SEMI 0
#define FIREMODE_AUTO 1
#define MAX_PELLETS 16
#define SCOPE_NONE 0
#define SCOPE_LOW 1
#define SCOPE_MEDIUM 2
#define SCOPE_HIGH 3
#define BUY_CATEGORY_NONE 0
#define BUY_CATEGORY_PISTOLS 1
#define BUY_CATEGORY_SHOTGUNS 2
#define BUY_CATEGORY_SMG 3
#define BUY_CATEGORY_RIFLES 4
#define BUY_CATEGORY_MACHINEGUNS 5
#define BUY_CATEGORY_EQUIPMENT 6
#define BUY_TEAM_T 1
#define BUY_TEAM_CT 2

struct Gun {
	JQuad* mHandQuad;
	JQuad* mGroundQuad;
	int mId;
	int mDelay;
	int mDamage;
	float mSpread;
	int mClip;
	int mNumClips;
	int mReloadDelay;
	float mSpeed;
	float mBulletSpeed;
	float mViewAngle;
	int mCost;
	int mType;
	int mFireMode;
	int mPellets;
	int mScope;
	int mBuyCategory;
	int mBuyTeams;
	char mName[15];
	JSample* mFireSound;
	JSample* mReloadSound;
	JSample* mDryFireSound;

	int mAmmoBarWidth;
};

//------------------------------------------------------------------------------------------------

class GunObject
{
private:
	static JRenderer* mRenderer;

protected:

public:
	float mAngle;
	float mSpeed;
	float mRotation;

	float mX;
	float mY;
	float mOldX;
	float mOldY;
	Gun *mGun;
	int mClipAmmo;
	int mRemainingAmmo;
	bool mOnGround;
	//bool mSpawned;

	bool mIsOnline;

	GunObject(Gun *gun, int clipammo, int remainingammo);
	virtual ~GunObject();

	virtual void Update(float dt);
	void Render(float x, float y);

	void SetTotalRotation(float theta);
	//void Reset();
};
#endif
