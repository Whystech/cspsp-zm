
#include "Person.h"
#include "Globals.h"

JRenderer* Person::mRenderer = NULL;
JSoundSystem* Person::mSoundSystem = NULL;

//------------------------------------------------------------------------------------------------
Person::Person(JQuad* quads[], JQuad* deadquad, std::vector<Bullet*>* bullets, std::vector<GunObject*>* guns, int team, char* name, int movementstyle)
{
	mRenderer = JRenderer::GetInstance();
	SetQuads(quads,deadquad);
	mX = 0.0f;
	mY = 0.0f;
	mRenderScale = 1.0f;
	mOldX = 0.0f;
	mOldY = 0.0f;
	mWalkX = 0.0f;
	mWalkY = 0.0f;
	mSpeed = 0.0f;
	mAngle = 0.0f;
	mMaxSpeed = 0.0f;
	mMovementSpeedMultiplier = 1.0f;

	mMoveState = NOTMOVING;
	mState = DEAD;
	//mState = 0;
	mStateTime = 0.0f;
	mHealth = team == T ? gPlayerConfig.tSpawnHealth : gPlayerConfig.ctSpawnHealth;
	mArmor = team == T ? gPlayerConfig.tSpawnArmor : gPlayerConfig.ctSpawnArmor;
	mMoney = gPlayerConfig.startingMoney;
	mRecoilAngle = 0.0f;
	SetTotalRotation(M_PI_2);
	//mRotation = 0.0f;
	//mFacingAngle = M_PI_2;
	//mLastFireAngle = 0.0f;
	mTeam = team;
	mOriginalTeam = team;
	strncpy(mName,name,25);
	mName[25] = '\0';
	mMovementStyle = movementstyle;
	mNumKills = 0;
	mNumDeaths = 0;
	mSoundId = -1;
	mNumDryFire = 0;
	/*mGuns[PRIMARY] = NULL;
	mGuns[SECONDARY] = NULL;
	mGuns[KNIFE] = NULL;
	mGuns[mGunIndex] = NULL;*/
	for (int i=0; i<5; i++) {
		mGuns[i] = NULL;
	}
	mKnifeGun = NULL;
	mGunIndex = KNIFE;
	mCorpseQuad = NULL;
	mCorpseX = 0.0f;
	mCorpseY = 0.0f;
	mCorpseRotation = 0.0f;
	mHasCorpse = false;

	mFadeTime = 0.0f;
	mStepTime = 0.0f;
	mIsActive = true;
	mBullets = bullets;
	mGunObjects = guns;

	mPing = 0.0f;
	mIsPlayerOnline = false;
	mIsFiring = false;
	mHasFired = false;

	mIsFlashed = false;
	mFlashTime = 0.0f;
	mFlashIntensity = 0.0f;

	mIsInBuyZone = false;

	mNode = NULL;
	mTargetNode = NULL;

	for (int i=0; i<ANIM_COUNT; i++) {
		mAnimations[i] = NULL;
	}

	for (int i=ANIM_PRIMARY; i<ANIM_COUNT; i++) {
		mAnimations[i] = new Animation(gKeyFrameAnims[i],true,false);
	}
	for (int i=0; i<3; i++) mWeaponAnimations[i] = new Animation(gKeyFrameAnims[ANIM_PRIMARY],true,false);

	mCurrentAnimation = mAnimations[1];

	mKeyFrame = KeyFrame();
	mKeyFrame.angles[BODY] = 0.4f;
	mKeyFrame.angles[RIGHTARM] = 0.0f;
	mKeyFrame.angles[RIGHTHAND] = 0.0f;
	mKeyFrame.angles[LEFTARM] = 0.0f;
	mKeyFrame.angles[LEFTHAND] = 0.0f;
	mKeyFrame.angles[GUN] = 0.0f;
	mKeyFrame.duration = 10;

	mWalkState = WALK1;
	mWalkTime = 0.0f;
	mWalkAngle = 0.0f;

	mMuzzleFlashType = 0;
	mMuzzleFlashFrame = 0;
	mMuzzleFlashAngle = 0.0f;
	mMuzzleFlashTime = 0.0f;

	mRadarTime = 0.0f;
	mRadarX = 0.0f;
	mRadarY = 0.0f;

	mHasFlag = false;

	mInvincibleTime = 0.0f;
	/*JTexture* texture = mRenderer->LoadTexture("gfx/playerparts.png");
	mPartQuads[BODY] = new JQuad(texture,0,0,32,16);
	mPartQuads[BODY]->SetHotSpot(16,8);

	mPartQuads[5] = new JQuad(texture,16,16,16,16);
	mPartQuads[5]->SetHotSpot(8,8);

	mPartQuads[RIGHTARM] = new JQuad(texture,0,16,16,8);
	mPartQuads[RIGHTARM]->SetHotSpot(4.5f,3.5f);

	mPartQuads[RIGHTHAND] = new JQuad(texture,0,24,16,8);
	mPartQuads[RIGHTHAND]->SetHotSpot(3.5f,3.5f);

	mPartQuads[LEFTARM] = new JQuad(texture,0,16,16,8);
	mPartQuads[LEFTARM]->SetHotSpot(4.5f,3.5f);
	mPartQuads[LEFTARM]->SetVFlip(true);

	mPartQuads[LEFTHAND] = new JQuad(texture,0,24,16,8);
	mPartQuads[LEFTHAND]->SetHotSpot(3.5f,3.5f);
	mPartQuads[LEFTHAND]->SetVFlip(true);*/
}

//------------------------------------------------------------------------------------------------
Person::~Person()
{
	//mPeople.clear();

	/*if (mGuns[PRIMARY])
		delete mGuns[PRIMARY];

	if (mGuns[SECONDARY])
		delete mGuns[SECONDARY];

	if (mGuns[KNIFE])
		delete mGuns[KNIFE];*/
	for (int i=0; i<5; i++) {
		if (mGuns[i]) {
			delete mGuns[i];
		}
	}
	if (mKnifeGun != NULL) {
		delete mKnifeGun;
		mKnifeGun = NULL;
	}
	for (int i=0; i<ANIM_COUNT; i++) {
		if (mAnimations[i] != NULL) {
			delete mAnimations[i];
		}
	}
	for (int i=0; i<3; i++) delete mWeaponAnimations[i];

	//delete mKeyFrame;

	mNodes.clear();
	
	if (mSoundId != -1) {
		mSoundSystem->StopSample(mSoundId);
		mSoundId = -1;
	}
	//JGERelease();
}

//------------------------------------------------------------------------------------------------
void Person::PreUpdate(float dt)
{
	if (mState == DEAD) {
		return;
	}

	float dx = mOldX-mX;
	float dy = mOldY-mY;

	//UpdateAngle(mWalkAngle,mAngle, 0.01*dt);
	float speed = sqrtf(dx*dx+dy*dy)/dt;
	if (speed > 0.15f) speed = 0.15f;
	if (mMoveState == NOTMOVING || speed < 0.03f) {
		if (mWalkTime > 0.0f) {
			mWalkTime -= dt;
			if (mWalkTime < 0.0f) {
				mWalkTime = 0.0f;
				mWalkState = (mWalkState+2)%4;
			}
		}
	}
	else {//if (mMoveState == MOVING) {
		//float speed = sqrtf(dx*dx+dy*dy)/dt;
		if (mWalkState == WALK1 || mWalkState == WALK3) {
			mWalkTime += dt*speed/0.1f;
			if (mWalkTime >= WALKTIME) {
				mWalkTime = WALKTIME-(mWalkTime-WALKTIME);
				mWalkState = (mWalkState+1)%4;
			}
		}
		else {
			mWalkTime -= dt*speed/0.1f;
			if (mWalkTime <= 0) {
				mWalkTime = -mWalkTime;
				mWalkState = (mWalkState+1)%4;
			}
		}
	}
	dx = mX-mWalkX;
	dy = mY-mWalkY;
	float d = sqrtf(dx*dx+dy*dy)/dt;

	if (d > 2+rand()%2) {
		mWalkX = mX;
		mWalkY = mY;
		//gSfxManager->PlaySample(gWalkSounds[rand()%2],mX,mY);
	}
	else if (speed > 0.06f) {
		mStepTime += dt;
		if (mStepTime > 480.0f + rand()%50 - 25.0f) {
			gSfxManager->PlaySample(gWalkSounds[rand()%2],mX,mY);
			mStepTime = 0.0f;
		}
	}
	//if (fabs(dx) >= EPSILON || fabs(dy) >= EPSILON) {
		/*if (speed > 0.06f) {
			mStepTime += dt;
			if (mStepTime > 480.0f + rand()%50 - 25.0f) {
				gSfxManager->PlaySample(gWalkSounds[rand()%2],mX,mY);
				mStepTime = 0.0f;
			}
		}*/
	//}
}

//------------------------------------------------------------------------------------------------
void Person::Update(float dt)
{
	if (mGuns[mGunIndex] == NULL) {
		printf("isPlayerOnline: %i\n",mIsPlayerOnline);
		printf("invalid gunindex: %i\n",mGunIndex);
		return;
	}
	mStateTime += dt;
	if (mState == DEAD) {
		// Keep dead bodies visible instead of fading out immediately.
		mFadeTime = 1000.0f;
		return;
	}

	mWalkAngle = mAngle;

	if (mIsActive) {
		if (mMoveState == NOTMOVING) {
			if (!mIsPlayerOnline) {
				mSpeed -= gPlayerConfig.deceleration*dt;
				if (mSpeed < 0) {
					mSpeed = 0;
				}
			}
			mStepTime = 240.0f;
		}
		else if (mMoveState == MOVING) {
			if (!mIsPlayerOnline) {
				mSpeed += gPlayerConfig.acceleration*dt;
				if (mSpeed > mMaxSpeed) {
					mSpeed = mMaxSpeed;
				}
			}
			if (mRecoilAngle < mGuns[mGunIndex]->mGun->mSpread*0.5f) { //HERE
				mRecoilAngle += mGuns[mGunIndex]->mGun->mSpread/50.0f*dt;
				if (mRecoilAngle > mGuns[mGunIndex]->mGun->mSpread*0.5f) {
					mRecoilAngle = mGuns[mGunIndex]->mGun->mSpread*0.5f;
				}
			}
		}
	}

	if (!mIsPlayerOnline) {
		mOldX = mX;
		mOldY = mY;
		mX += mSpeed * cosf(mAngle) * dt;
		mY += mSpeed * sinf(mAngle) * dt;
	}

	//JSprite::Update(dt);

	if (mGuns[mGunIndex]->mGun->mPellets > 1) {
		mRecoilAngle = mGuns[mGunIndex]->mGun->mSpread;
	}

	mLastFireAngle = mRotation;
	if (mState == NORMAL) {
		if (mGuns[mGunIndex]->mGun->mPellets == 1) {
			mRecoilAngle -= mGuns[mGunIndex]->mGun->mSpread/100.0f*dt;
			if (mRecoilAngle < 0) {
				mRecoilAngle = 0.0f;
			}
		}
	}
	else if (mState == ATTACKING) {
		if (mMoveState == NOTMOVING) {
			mRecoilAngle += mGuns[mGunIndex]->mGun->mSpread/500.0f*dt;
		}
		else if (mMoveState == MOVING) {
			mRecoilAngle += mGuns[mGunIndex]->mGun->mSpread/50.0f*dt;
		}

		if (mGuns[mGunIndex]->mGun->mScope >= SCOPE_MEDIUM) {
			mRecoilAngle = mGuns[mGunIndex]->mGun->mSpread;
		}
		if (mRecoilAngle > mGuns[mGunIndex]->mGun->mSpread) {
			mRecoilAngle = mGuns[mGunIndex]->mGun->mSpread;
		}
		if (mRecoilAngle*500.0f >= 0.1f && mStateTime < 100.0f) {
			mLastFireAngle += (rand()%(int)ceilf(mRecoilAngle*500.0f))/1000.0f-(mRecoilAngle/4.0f);
		}

		if (mStateTime >= mGuns[mGunIndex]->mGun->mDelay) {
			if (mGunIndex == GRENADE) {
				mStateTime = mGuns[mGunIndex]->mGun->mDelay;
			}
			else {
				SetState(NORMAL);
			}
		}
	}
	else if (mState == RELOADING) {
		if (mStateTime >= mGuns[mGunIndex]->mGun->mReloadDelay) {
			mGuns[mGunIndex]->mRemainingAmmo -= (mGuns[mGunIndex]->mGun->mClip-mGuns[mGunIndex]->mClipAmmo);
			mGuns[mGunIndex]->mClipAmmo = mGuns[mGunIndex]->mGun->mClip;
			if (mGuns[mGunIndex]->mRemainingAmmo < 0) {
				mGuns[mGunIndex]->mClipAmmo = mGuns[mGunIndex]->mGun->mClip + mGuns[mGunIndex]->mRemainingAmmo ;
				mGuns[mGunIndex]->mRemainingAmmo = 0;
			}
			SetState(NORMAL);
		}
		if (mGuns[mGunIndex]->mGun->mPellets == 1) {
			mRecoilAngle -= mGuns[mGunIndex]->mGun->mSpread/100.0f*dt;
			if (mRecoilAngle < 0.0f) {
				mRecoilAngle = 0.0f;
			}
		}
	}
	else if (mState == DRYFIRING) {
		if (mGunIndex == PRIMARY) {
			if (mStateTime >= 250.0f) {
				SetState(NORMAL);
				mNumDryFire++;
			}
		}
		else if (mGunIndex == SECONDARY) {
			SetState(NORMAL);
			mNumDryFire++;
		}
		else if (mGunIndex == KNIFE) {
			if (mStateTime >= mGuns[mGunIndex]->mGun->mDelay) {
				SetState(NORMAL);
			}
		}
	}
	else if (mState == SWITCHING) {
		if (mGunIndex == PRIMARY || mGunIndex == SECONDARY) {
			float delay = mGuns[mGunIndex]->mGun->mDelay*0.75f;
			if (delay < 150.0f) delay = 150.0f;
			if (mStateTime >= delay) {
				SetState(NORMAL);
			}
		}
		else {
			if (mStateTime >= 150.0f) {
				SetState(NORMAL);
			}
		}
	}

	if (!mIsPlayerOnline) {
		if (mNumDryFire > 3) {
			Reload();
		}
	}

	if (mIsFlashed) {
		mFlashTime -= dt/mFlashIntensity;
		if (mFlashTime < 0.0f) {
			mFlashTime = 0.0f;
			mIsFlashed = false;
		}
	}

	mMuzzleFlashTime -= dt;
	if (mMuzzleFlashTime < 0.0f) {
		mMuzzleFlashTime = 0.0f;
	}

	mRadarTime -= dt;
	if (mRadarTime < 0.0f) {
		mRadarTime = 0.0f;
	}
	mCurrentAnimation->Update(dt,mKeyFrame);

	mInvincibleTime -= dt;
	if (mInvincibleTime < 0.0f) {
		mInvincibleTime = 0.0f;
	}
}


//------------------------------------------------------------------------------------------------
void Person::Render(float x, float y)
{
	float offsetX = (x-SCREEN_WIDTH_2);
	float offsetY = (y-SCREEN_HEIGHT_2);
	if (mState != DEAD) {	
		float scale = mRenderScale;
		float rotation = mRotation+M_PI_2;
		float centerx = mX-offsetX-4*scale*cosf(rotation);
		float centery = mY-offsetY-4*scale*sinf(rotation);
		float x = centerx;
		float y = centery;

		float offsetangle = mWalkTime/WALKTIME*(M_PI_2);
		float offset = 7*scale*sinf(offsetangle);
		if (mWalkState == WALK3 || mWalkState == WALK4) {
			//offset = -offset;
			mQuads[LEGS]->SetHFlip(true);	
		}
		else {
			mQuads[LEGS]->SetHFlip(false);	
		}

		float dx = offset*cosf(mAngle);
		float dy = offset*sinf(mAngle);
		float x2 = x+dx;
		float y2 = y+dy;

		//mRenderer->FillCircle(x,y,12,ARGB(100,0,0,0));
		/*rotation = mAngle;
		if (rotation < 2*M_PI) {
			rotation += 2*M_PI;
		}*/
		
		mRenderer->RenderQuad(mQuads[LEGS],x,y,mWalkAngle-M_PI_2,scale,offset/7);
		//mQuads[RIGHTLEG]->SetVFlip(true);
		//mRenderer->RenderQuad(mQuads[LEFTLEG],x+4*cosf(mWalkAngle-M_PI_2),y+4*sinf(mWalkAngle-M_PI_2),mWalkAngle-M_PI_2,1.0f,-offset/7);
		//mRenderer->FillCircle(x+4*cosf(mWalkAngle-M_PI_2) - offset*cosf(mWalkAngle),y+4*sinf(mWalkAngle-M_PI_2) - offset*sinf(mWalkAngle),3.0f,ARGB(255,0,0,0));
		//mRenderer->FillCircle(x+4*cosf(mWalkAngle+M_PI_2) + offset*cosf(mWalkAngle),y+4*sinf(mWalkAngle+M_PI_2) + offset*sinf(mWalkAngle),3.0f,ARGB(255,0,0,0));
		

		if (mGuns[mGunIndex] != NULL) {
			//mRenderer->RenderQuad(mGuns[mGunIndex]->mGun->mHandQuad,mX-offsetX,mY-offsetY,mLastFireAngle);
			if (mGunIndex == KNIFE && (mState == ATTACKING || mState == DRYFIRING)) {
				float angle = 0;
				if (mStateTime < (mGuns[KNIFE]->mGun->mDelay*0.2f)) {
					angle = mStateTime/(mGuns[KNIFE]->mGun->mDelay*0.2f);
				}
				else if (mStateTime >= (mGuns[KNIFE]->mGun->mDelay*0.2f)) {
					angle = (mGuns[KNIFE]->mGun->mDelay-mStateTime)/(mGuns[KNIFE]->mGun->mDelay*0.8f);
				}

				//mGuns[mGunIndex]->mGun->mHandQuad->SetColor(ARGB(200,255,255,255));
				//mRenderer->RenderQuad(mGuns[mGunIndex]->mGun->mHandQuad,mX-offsetX,mY-offsetY,mLastFireAngle+angle);
				//mGuns[mGunIndex]->mGun->mHandQuad->SetColor(ARGB(255,255,255,255));
			}
		}

		//mRenderer->RenderQuad(mQuads[mGunIndex],mX-offsetX,mY-offsetY,mRotation);

		rotation = mRotation+M_PI_2;
		centerx = mX-offsetX-5*scale*cosf(rotation);
		centery = mY-offsetY-5*scale*sinf(rotation);
		x = centerx;
		y = centery;
		dx = 10*scale*cosf(mRotation+mKeyFrame.angles[BODY]);
		dy = 10*scale*sinf(mRotation+mKeyFrame.angles[BODY]);
		x2 = x+dx;
		y2 = y+dy;

		if (mHasFlag) {
			float tempx = x-10*cosf(rotation);
			float tempy = y-10*sinf(rotation);
			if (mTeam == T) {
				gFlagQuad->SetColor(ARGB(255,153,204,255));
			}
			else {
				gFlagQuad->SetColor(ARGB(255,255,64,64));
			}
			mRenderer->RenderQuad(gFlagQuad,tempx,tempy,mRotation+mKeyFrame.angles[BODY]-M_PI_4*0.6f);
		}

		mRenderer->RenderQuad(mQuads[BODY],x,y,mRotation+mKeyFrame.angles[BODY],scale,scale);
		//mRenderer->DrawLine(x,y,x2,y2,ARGB(255,255,255,255));
		x = x2;
		y = y2;
		x2 = x+8*scale*cosf(rotation+mKeyFrame.angles[LEFTARM]);
		y2 = y+8*scale*sinf(rotation+mKeyFrame.angles[LEFTARM]);
		mRenderer->RenderQuad(mQuads[LEFTARM],x,y,mRotation+mKeyFrame.angles[LEFTARM],scale,scale);
		//mRenderer->DrawLine(x,y,x2,y2,ARGB(255,255,255,255));
		x = x2;
		y = y2;
		//x2 = x+10*cosf(mRotation+mKeyFrame.angles[LEFTHAND]);
		//y2 = y+10*sinf(mRotation+mKeyFrame.angles[LEFTHAND]);
		mRenderer->RenderQuad(mQuads[LEFTHAND],x,y,mRotation+mKeyFrame.angles[LEFTHAND],scale,scale);
		//mRenderer->DrawLine(x,y,x2,y2,ARGB(255,255,255,255));
		x = centerx;
		y = centery;
		x2 = x-dx;
		y2 = y-dy;
		//mRenderer->DrawLine(x,y,x2,y2,ARGB(255,255,255,255));
		x = x2;
		y = y2;
		x2 = x+8*scale*cosf(rotation+mKeyFrame.angles[RIGHTARM]);
		y2 = y+8*scale*sinf(rotation+mKeyFrame.angles[RIGHTARM]);
		mRenderer->RenderQuad(mQuads[RIGHTARM],x,y,mRotation+mKeyFrame.angles[RIGHTARM],scale,scale);
		//mRenderer->DrawLine(x,y,x2,y2,ARGB(255,255,255,255));
		x = x2;
		y = y2;
		x2 = x+10*scale*cosf(rotation+mKeyFrame.angles[RIGHTHAND]);
		y2 = y+10*scale*sinf(rotation+mKeyFrame.angles[RIGHTHAND]);
		mRenderer->RenderQuad(mQuads[RIGHTHAND],x,y,mRotation+mKeyFrame.angles[RIGHTHAND],scale,scale);
		//mRenderer->DrawLine(x,y,x2,y2,ARGB(255,255,255,255));
		x = x2;
		y = y2;
		x += scale*(mGuns[mGunIndex]->mGun->mSpriteOffsetX*cosf(mFacingAngle)-mGuns[mGunIndex]->mGun->mSpriteOffsetY*sinf(mFacingAngle));
		y += scale*(mGuns[mGunIndex]->mGun->mSpriteOffsetX*sinf(mFacingAngle)+mGuns[mGunIndex]->mGun->mSpriteOffsetY*cosf(mFacingAngle));
		//x2 = x+11*cosf(mRotation+mKeyFrame.angles[GUN]);
		//y2 = y+11*sinf(mRotation+mKeyFrame.angles[GUN]);
		if (mTeam != T || mGunIndex != KNIFE || mGuns[mGunIndex]->mGun->mId == ZOMBIECLAWS) {
			mRenderer->RenderQuad(mGuns[mGunIndex]->mGun->mHandQuad,x,y,mLastFireAngle+mKeyFrame.angles[GUN],scale,scale);
		}

		if (mMuzzleFlashTime > 0.0f) {
			/*x2 = x+10*cosf(rotation+mKeyFrame.angles[GUN]);
			y2 = y+10*sinf(rotation+mKeyFrame.angles[GUN]);
			x = x2;
			y = y2;*/
			//int alpha = mMuzzleFlashTime/100.0f*255;
			JQuad* muzzleFlash = GetMuzzleFlashQuad(mMuzzleFlashType,mMuzzleFlashFrame);
			float animationOffsetX, animationOffsetY, weaponAngle;
			GetWeaponAnimationTransform(0.0f,0.0f,animationOffsetX,animationOffsetY,weaponAngle);
			MuzzlePositionConfig muzzle = gMuzzlePositionConfigs[mGuns[mGunIndex]->mGun->mId];
			x += scale*(muzzle.forward*cosf(weaponAngle)-muzzle.sideways*sinf(weaponAngle));
			y += scale*(muzzle.forward*sinf(weaponAngle)+muzzle.sideways*cosf(weaponAngle));
			if (muzzleFlash != NULL) mRenderer->RenderQuad(muzzleFlash,x,y,weaponAngle-M_PI_2,scale,scale);
		}

		mRenderer->RenderQuad(mQuads[5],centerx,centery,mRotation,scale,scale);
		//mRenderer->DrawLine(x,y,x2,y2,ARGB(255,255,255,255));
		/*mAnimationAngles[BODY] = 0.0f;
		mAnimationAngles[RIGHTARM] = 0.0f;
		mAnimationAngles[RIGHTHAND] = 0.0f;
		mAnimationAngles[LEFTARM] = 0.0f;
		mAnimationAngles[LEFTHAND] = 0.0f;
		mAnimationAngles[GUN] = 0.0f;*/
		//mRenderer->DrawCircle(mOldX-offsetX,mOldY-offsetY,16,ARGB(255,0,0,0));
		//mRenderer->DrawCircle(mX-offsetX,mY-offsetY,16,ARGB(255,255,255,255));

	}
	else {
		if (mFadeTime > 0) {
			mDeadQuad->SetColor(ARGB((int)(mFadeTime*(255.0f/1000.0f)),255,255,255));
			mRenderer->RenderQuad(mDeadQuad,mX-offsetX,mY-offsetY,mRotation,mRenderScale,mRenderScale);
		}
	}
}

void Person::RenderCorpse(float x, float y)
{
	if (!mHasCorpse || mCorpseQuad == NULL) return;
	float offsetX = (x-SCREEN_WIDTH_2);
	float offsetY = (y-SCREEN_HEIGHT_2);
	mCorpseQuad->SetColor(ARGB(255,255,255,255));
	mRenderer->RenderQuad(mCorpseQuad,mCorpseX-offsetX,mCorpseY-offsetY,mCorpseRotation);
}

//------------------------------------------------------------------------------------------------
void Person::Move(float speed, float angle)
{
	if (!mIsActive) return;
	SetMoveState(MOVING);
	mMaxSpeed = speed*mMovementSpeedMultiplier*mGuns[mGunIndex]->mGun->mSpeed;
	if (mMovementStyle == RELATIVE1) {
		mAngle = mFacingAngle+angle;
	}
	else if (mMovementStyle == ABSOLUTE1) {
		mAngle = angle-M_PI_2;
	}

	if (mAngle > M_PI) {
		mAngle -= 2*M_PI;
	}
	if (mAngle < -M_PI) {
		mAngle += 2*M_PI;
	}
}

//------------------------------------------------------------------------------------------------
std::vector<Bullet*> Person::Fire()
{
	std::vector<Bullet*> bullets;
	if (!mIsActive) return bullets;
	if (mState == NORMAL) {
		mIsFiring = true;
		mHasFired = true;

		if (mGunIndex == KNIFE) {
			SetState(ATTACKING);
			gSfxManager->PlaySample(mGuns[mGunIndex]->mGun->mFireSound,mX,mY);
			//return true;
		}
		else if (mGunIndex == GRENADE) {
			if (mGuns[mGunIndex]->mClipAmmo != 0)  {
				SetState(ATTACKING);
				gSfxManager->PlaySample(mGuns[mGunIndex]->mGun->mFireSound,mX,mY);
			}
		}
		else {
			if (mGuns[mGunIndex]->mClipAmmo != 0)  {
				Bullet* bullet;
				int gunId = mGuns[mGunIndex]->mGun->mId;
				int launchOrigin = gunId >= 0 && gunId < MAX_GUNS ? gProjectileLaunchOrigins[gunId] : -1;
				bool shoulderWeapon = launchOrigin < 0 ? mGuns[mGunIndex]->mGun->mProjectileType == PROJECTILE_ROCKET : launchOrigin == 1;
				float muzzleForward = (shoulderWeapon ? 28.0f : 24.0f)+mGuns[mGunIndex]->mGun->mSpriteOffsetX;
				float muzzleSide = (shoulderWeapon ? 8.0f : 0.0f)+mGuns[mGunIndex]->mGun->mSpriteOffsetY;
				float h = muzzleForward*sinf(mFacingAngle)+muzzleSide*cosf(mFacingAngle);
				float w = muzzleForward*cosf(mFacingAngle)-muzzleSide*sinf(mFacingAngle);
				float theta = mFacingAngle;
				float speed = 0.3f*mGuns[mGunIndex]->mGun->mBulletSpeed;
				if (mGuns[mGunIndex]->mGun->mPellets > 1) {
					int pelletCount = mGuns[mGunIndex]->mGun->mPellets;
					theta -= mGuns[mGunIndex]->mGun->mSpread/2;
					float step = mGuns[mGunIndex]->mGun->mSpread/(pelletCount-1);
					int randomSteps = pelletCount == 4 ? 10 : 11;
					for (int i=0; i<pelletCount; i++) {
						theta += (rand()%randomSteps)/100.0f-0.05f;
						if (mGuns[mGunIndex]->mGun->mProjectileType == PROJECTILE_ROCKET) {
							bullet = new Rocket(mX+w,mY+h,mX+w,mY+h,theta,speed,this);
						}
						else {
							bullet = new Bullet(mX+w,mY+h,mX+w,mY+h,theta,speed,abs(mGuns[mGunIndex]->mGun->mDamage+rand()%17-8),this);
						}
						bullets.push_back(bullet);
						mBullets->push_back(bullet);
						theta += step;
					}
				}
				else {
					if (mRecoilAngle*1000.0f >= 0.1f) {
						theta += (rand()%(int)ceilf(mRecoilAngle*1000.0f))/1000.0f-(mRecoilAngle*0.5f);
						//theta = mFacingAngle + (rand()%100)/400.0f-0.125f;
					}
					if (mGuns[mGunIndex]->mGun->mProjectileType == PROJECTILE_ROCKET) {
						bullet = new Rocket(mX+w,mY+h,mX+w,mY+h,theta,speed,this);
					}
					else {
						bullet = new Bullet(mX+w,mY+h,mX+w,mY+h,theta,speed,abs(mGuns[mGunIndex]->mGun->mDamage+rand()%17-8),this);
					}
					bullets.push_back(bullet);
					mBullets->push_back(bullet);
				}
				gParticleEngine->GenerateParticles(BULLETSHELL,mX+10*cosf(mFacingAngle),mY+10*sinf(mFacingAngle),gEffectsConfig.shellParticleCount);
				SetState(ATTACKING);
				mGuns[mGunIndex]->mClipAmmo--;
				//JSample *test = mEngine->LoadSample("sfx/m249.wav");
				gSfxManager->PlaySample(mGuns[mGunIndex]->mGun->mFireSound,mX,mY);

				mMuzzleFlashTime = gEffectsConfig.muzzleFlashLifetime;
				mMuzzleFlashAngle = mFacingAngle;
				mMuzzleFlashType = mGuns[mGunIndex]->mGun->mMuzzleFlashType;
				mMuzzleFlashFrame = rand()%MUZZLE_FLASH_FRAMES;

				mRadarTime = 2000.0f;
				mRadarX = mX;
				mRadarY = mY;
				//return true;
			}
			else {
				SetState(DRYFIRING);
				gSfxManager->PlaySample(mGuns[mGunIndex]->mGun->mDryFireSound,mX,mY);
				//return;
			}
		}
	}
	return bullets;
	//return false;
}

//------------------------------------------------------------------------------------------------
std::vector<Bullet*> Person::StopFire()
{
	std::vector<Bullet*> bullets;
	if (!mIsActive) return bullets;

	mIsFiring = false;
	mHasFired = false;

	if (mState == ATTACKING) {
		if (mGunIndex == GRENADE) {
			if (mGuns[mGunIndex]->mClipAmmo != 0)  {
				float h = 24*sinf(mFacingAngle);
				float w = 24*cosf(mFacingAngle);
				float theta = mFacingAngle;

				float speed = 0.2f * mStateTime/(float)mGuns[mGunIndex]->mGun->mDelay;
				int type = HE;

				if (mGuns[mGunIndex]->mGun->mId == 25) { //FLASH
					type = FLASH;
				}
				else if (mGuns[mGunIndex]->mGun->mId == 26) { //HE
					type = HE;
				}
				else if (mGuns[mGunIndex]->mGun->mId == 27) { //SMOKE
					type = SMOKE;
				}

				Grenade* grenade = new Grenade(mX+w,mY+h,mX,mY,theta,speed,this,type);
				bullets.push_back(grenade);
				mBullets->push_back(grenade);

				SetState(NORMAL);
				gSfxManager->PlaySample(gFireInTheHoleSound,mX,mY);
				mGuns[mGunIndex]->mClipAmmo--;

				delete mGuns[mGunIndex];
				mGuns[mGunIndex] = NULL;
				SwitchNext();
			}
		}
	}
	return bullets;
}

//------------------------------------------------------------------------------------------------
bool Person::Reload()
{
	if (mGunIndex == KNIFE || mGunIndex == GRENADE) return false;
	if (mState != RELOADING && mGuns[mGunIndex]->mClipAmmo != mGuns[mGunIndex]->mGun->mClip && mGuns[mGunIndex]->mRemainingAmmo != 0) {
		SetState(RELOADING);
		mSoundId = gSfxManager->PlaySample(mGuns[mGunIndex]->mGun->mReloadSound,mX,mY);
		mNumDryFire = 0;
		return true;
	}
	return false;
}

//------------------------------------------------------------------------------------------------
void Person::Switch(int index)
{
	if (mGuns[index] != NULL) {
		mGunIndex = index;

		if (mState == RELOADING) {
			//SetState(NORMAL);
			if (mSoundId != -1) {
				mSoundSystem->StopSample(mSoundId);
				mSoundId = -1;
			}
		}
		else if (mState == ATTACKING) {
			//SetState(NORMAL);
		}

		mRecoilAngle = 0.0f;
		mNumDryFire = 0;
		SetState(SWITCHING);
	}
}

//------------------------------------------------------------------------------------------------
void Person::SwitchNext()
{
	if (mState == RELOADING) {
		//SetState(NORMAL);
		if (mSoundId != -1) {
			mSoundSystem->StopSample(mSoundId);
			mSoundId = -1;
		}
	}
	else if (mState == ATTACKING) {
		//SetState(NORMAL);
	}

	int gunindex = mGunIndex;
	for (int i=0; i<5; i++) {
		gunindex++;
		if (gunindex > 4) {
			gunindex = 0;
		}
		if (mGuns[gunindex] != NULL) break;
	}

	if (gunindex == mGunIndex) return;

	mGunIndex = gunindex;
	
	if (mGunIndex == PRIMARY) {
		gSfxManager->PlaySample(gDeploySound,mX,mY);
	}

	/*if (mGunIndex == PRIMARY) {
		if (mGuns[SECONDARY] != NULL) {
			mGunIndex = SECONDARY;
		}
		else {
			mGunIndex = KNIFE;
		}
	}
	else if (mGunIndex == SECONDARY) {
		if (mGuns[KNIFE] != NULL) {
			mGunIndex = KNIFE;
		}
		else {
			mGunIndex = PRIMARY;
			gSfxManager->PlaySample(gDeploySound,mX,mY);
		}
	}
	else if (mGunIndex == KNIFE) {
		if (mGuns[PRIMARY] != NULL) {
			mGunIndex = PRIMARY;
			gSfxManager->PlaySample(gDeploySound,mX,mY);
		}
		else if (mGuns[SECONDARY] != NULL) {
			mGunIndex = SECONDARY;
		}
	}*/

	mRecoilAngle = 0.0f;
	mNumDryFire = 0;
	SetState(SWITCHING);
}

//------------------------------------------------------------------------------------------------
void Person::SetKnifeForTeam(int team)
{
	int meleeWeaponId = (team == T) ? ZOMBIECLAWS : 0;

	if (mGuns[KNIFE] != NULL) {
		delete mGuns[KNIFE];
		mGuns[KNIFE] = NULL;
	}
	if (mKnifeGun != NULL) {
		delete mKnifeGun;
		mKnifeGun = NULL;
	}

	mKnifeGun = new Gun(gGuns[meleeWeaponId]);
	mKnifeGun->mHandQuad = gGunHandQuads[meleeWeaponId];
	mKnifeGun->mGroundQuad = gGunGroundQuads[meleeWeaponId];
	mGuns[KNIFE] = new GunObject(mKnifeGun, 0, 0);
	mGunIndex = KNIFE;
	if (mState == ATTACKING || mState == DRYFIRING) {
		SetAnimation(ANIM_KNIFE_SLASH);
		mCurrentAnimation->SetSpeed(1000.0f/mGuns[KNIFE]->mGun->mDelay);
	}
	else {
		SetAnimation(ANIM_KNIFE);
		mCurrentAnimation->SetSpeed(1.0f);
	}
}

//------------------------------------------------------------------------------------------------
bool Person::PickUp(GunObject* gunobject)
{
	if (mTeam == T) {
		return false;
	}
	if (gunobject->mGun->mType == PRIMARY) {
		if (mGuns[PRIMARY] == NULL) {
			mGuns[PRIMARY] = gunobject;
			gunobject->mOnGround = false;
			/*if (mState == RELOADING) {
				SetState(NORMAL);
				if (mSoundId != -1) {
					mSoundSystem->StopSample(mSoundId);
					mSoundId = -1;
				}
			}
			mGunIndex = PRIMARY;*/
			Switch(PRIMARY);
			gSfxManager->PlaySample(gPickUpSound, mX, mY);
			return true;
		}
	}
	else if (gunobject->mGun->mType == SECONDARY) {
		if (mGuns[SECONDARY] == NULL) {
			mGuns[SECONDARY] = gunobject;
			gunobject->mOnGround = false;
			if (mGuns[PRIMARY] == NULL) {
				//mGunIndex = SECONDARY;
				Switch(SECONDARY);
			}
			gSfxManager->PlaySample(gPickUpSound, mX, mY);
			return true;
		}
	}
	else if (gunobject->mGun->mType == KNIFE) {
		if (mGuns[KNIFE] == NULL) {
			mGuns[KNIFE] = gunobject;
			gunobject->mOnGround = false;
			return true;
		}
	}
	else if (gunobject->mGun->mType == GRENADE) {
		if (mGuns[GRENADE] == NULL) {
			mGuns[GRENADE] = gunobject;
			gunobject->mOnGround = false;
			if (mGuns[PRIMARY] == NULL && mGuns[SECONDARY] == NULL) {
				//mGunIndex = GRENADE;
				Switch(GRENADE);
			}
			gSfxManager->PlaySample(gPickUpSound, mX, mY);
			return true;
		}
	}
	return false;
}

//------------------------------------------------------------------------------------------------
bool Person::Drop(int index, float speed)
{
	if (index >= 5) return false;
	if (index == KNIFE) return false;

	GunObject* gunobject = mGuns[index];
	if (gunobject == NULL) return false;

	/*if (mState == RELOADING) {
		SetState(NORMAL);
		if (mSoundId != -1) {
			mSoundSystem->StopSample(mSoundId);
			mSoundId = -1;
		}
	}*/
	gunobject->mX = mX;
	gunobject->mY = mY;
	gunobject->mOldX = mX;
	gunobject->mOldY = mY;
	gunobject->mRotation = mRotation;
	gunobject->mAngle = mFacingAngle;
	gunobject->mSpeed = speed;
	gunobject->mOnGround = false;
	mGunObjects->push_back(gunobject);

	mGuns[index] = NULL;
	
	if (mGuns[PRIMARY] != NULL) {
		//mGunIndex = PRIMARY;
		Switch(PRIMARY);
	}
	else if (mGuns[SECONDARY] != NULL) {
		//mGunIndex = SECONDARY;
		Switch(SECONDARY);
	}
	else if (mGuns[GRENADE] != NULL) {
		//mGunIndex = GRENADE;
		Switch(GRENADE);
	}
	else if (mGuns[KNIFE] != NULL) {
		//mGunIndex = KNIFE;
		Switch(KNIFE);
	}

	//mRecoilAngle = 0.0f;
	//mNumDryFire = 0;
	return true;
}

//------------------------------------------------------------------------------------------------
void Person::RotateFacing(float theta)
{
	float thetaTemp = mRotation + theta;
	if (thetaTemp > M_PI*2.0f) {			// angles are in radian, so 2 PI is one full circle
		thetaTemp -= M_PI*2.0f;
	}
	else if (thetaTemp < 0) {
		thetaTemp += M_PI*2.0f;
	}
	mRotation = thetaTemp;
	thetaTemp = mFacingAngle + theta;
	if (thetaTemp > M_PI*2.0f) {			// angles are in radian, so 2 PI is one full circle
		thetaTemp -= M_PI*2.0f;
	}
	else if (thetaTemp < 0) {
		thetaTemp += M_PI*2.0f;
	}
	mFacingAngle = thetaTemp;
}

//------------------------------------------------------------------------------------------------
void Person::SetMoveState(int state)
{
	if (mMoveState != state) {
		mMoveState = state;
	}
}

//------------------------------------------------------------------------------------------------
void Person::SetState(int state)
{
	if (mState != state) {
		mState = state;
		mStateTime = 0;
		if (mState == NORMAL || mState == SWITCHING) {
			if (mGunIndex == PRIMARY) {
				SetWeaponAnimation(0,ANIM_PRIMARY);
			}
			else if (mGunIndex == SECONDARY) {
				SetWeaponAnimation(0,ANIM_SECONDARY);
			}
			else if (mGunIndex == KNIFE) {
				SetWeaponAnimation(0,ANIM_KNIFE);
			}
			else if (mGunIndex == GRENADE) {
				SetWeaponAnimation(0,ANIM_GRENADE);
			}
		}
		else if (mState == ATTACKING) {
			if (mGunIndex == PRIMARY) {
				SetWeaponAnimation(1,ANIM_PRIMARY_FIRE);
				//mCurrentAnimation->Reset();
				//mCurrentAnimation->Play();
				mCurrentAnimation->SetSpeed(1000.0f/mGuns[mGunIndex]->mGun->mDelay);
			}
			else if (mGunIndex == SECONDARY) {
				SetWeaponAnimation(1,ANIM_SECONDARY_FIRE);
				//mCurrentAnimation->Reset();
				//mCurrentAnimation->Play();
				mCurrentAnimation->SetSpeed(1000.0f/mGuns[mGunIndex]->mGun->mDelay);
			}
			else if (mGunIndex == KNIFE) {
				SetWeaponAnimation(1,ANIM_KNIFE_SLASH);
				//mCurrentAnimation->Reset();
				//mCurrentAnimation->Play();
				mCurrentAnimation->SetSpeed(1000.0f/mGuns[mGunIndex]->mGun->mDelay);
			}
			else if (mGunIndex == GRENADE) {
				SetWeaponAnimation(1,ANIM_GRENADE_PULLBACK);
			}
		}
		else if (mState == RELOADING) {
			if (mGunIndex == PRIMARY) {
				SetWeaponAnimation(2,ANIM_PRIMARY_RELOAD);
				mCurrentAnimation->SetSpeed(1000.0f/mGuns[mGunIndex]->mGun->mReloadDelay);
			}
			else if (mGunIndex == SECONDARY) {
				SetWeaponAnimation(2,ANIM_SECONDARY_RELOAD);
				mCurrentAnimation->SetSpeed(1000.0f/mGuns[mGunIndex]->mGun->mReloadDelay);
			}
		}
	}
	if (mState == SWITCHING) {
		if (mGunIndex == PRIMARY) {
			SetWeaponAnimation(0,ANIM_PRIMARY);
			//mCurrentAnimation->SetSpeed(100.0f/(mGuns[mGunIndex]->mGun->mDelay*0.75f));
		}
		else if (mGunIndex == SECONDARY) {
			SetWeaponAnimation(0,ANIM_SECONDARY);
			//mCurrentAnimation->SetSpeed(100.0f/(mGuns[mGunIndex]->mGun->mDelay*0.75f));
		}
		else if (mGunIndex == KNIFE) {
			SetWeaponAnimation(0,ANIM_KNIFE);
			mCurrentAnimation->SetSpeed(1);
		}
		else if (mGunIndex == GRENADE) {
			SetWeaponAnimation(0,ANIM_GRENADE);
			mCurrentAnimation->SetSpeed(1);
		}
		mKeyFrame.angles[GUN] = mCurrentAnimation->GetKeyFrame(0)->angles[GUN];
	}
}

//------------------------------------------------------------------------------------------------
void Person::SetWeaponAnimation(int action, int fallbackAnimation)
{
	Animation* animation = mAnimations[fallbackAnimation];
	if (mGuns[mGunIndex] != NULL && mGuns[mGunIndex]->mGun != NULL) {
		int gunId = mGuns[mGunIndex]->mGun->mId;
		if (gunId >= 0 && gunId < MAX_GUNS && gWeaponAnimationKeyFrames[gunId][action] != NULL) {
			mWeaponAnimations[action]->SetKeyFrameAnim(gWeaponAnimationKeyFrames[gunId][action]);
			animation = mWeaponAnimations[action];
		}
	}
	if (mCurrentAnimation != animation) {
		mCurrentAnimation->Reset();
		mCurrentAnimation = animation;
		mCurrentAnimation->Play();
	}
}

//------------------------------------------------------------------------------------------------
void Person::SetAnimation(int animation)
{
	if (mCurrentAnimation != mAnimations[animation]) {
		mCurrentAnimation->Reset();
		mCurrentAnimation = mAnimations[animation];
		mCurrentAnimation->Play();
	}
	else {
		//mCurrentAnimation->Reset();
		//mCurrentAnimation->Play();
	}
}

void Person::GetWeaponAnimationTransform(float pointX, float pointY, float& offsetX, float& offsetY, float& angle)
{
	offsetX = pointX*cosf(mFacingAngle)-pointY*sinf(mFacingAngle);
	offsetY = pointX*sinf(mFacingAngle)+pointY*cosf(mFacingAngle);
	angle = mFacingAngle;
	if (mGuns[mGunIndex] == NULL || mGuns[mGunIndex]->mGun == NULL) return;

	int gunId = mGuns[mGunIndex]->mGun->mId;
	KeyFrameAnim* assignedIdle = gunId >= 0 && gunId < MAX_GUNS ? gWeaponAnimationKeyFrames[gunId][0] : NULL;
	int idleAnimation = mGunIndex;
	KeyFrame* idleFrame = assignedIdle == NULL ? mAnimations[idleAnimation]->GetKeyFrame(0) : &assignedIdle->mKeyFrames[0];
	if (idleFrame == NULL) return;

	float rotation = mRotation+M_PI_2;
	float scale = mRenderScale;
	float currentX = -5.0f*scale*cosf(rotation)
		-10.0f*scale*cosf(mRotation+mKeyFrame.angles[BODY])
		+8.0f*scale*cosf(rotation+mKeyFrame.angles[RIGHTARM])
		+10.0f*scale*cosf(rotation+mKeyFrame.angles[RIGHTHAND])
		+scale*(mGuns[mGunIndex]->mGun->mSpriteOffsetX*cosf(mFacingAngle)-mGuns[mGunIndex]->mGun->mSpriteOffsetY*sinf(mFacingAngle));
	float currentY = -5.0f*scale*sinf(rotation)
		-10.0f*scale*sinf(mRotation+mKeyFrame.angles[BODY])
		+8.0f*scale*sinf(rotation+mKeyFrame.angles[RIGHTARM])
		+10.0f*scale*sinf(rotation+mKeyFrame.angles[RIGHTHAND])
		+scale*(mGuns[mGunIndex]->mGun->mSpriteOffsetX*sinf(mFacingAngle)+mGuns[mGunIndex]->mGun->mSpriteOffsetY*cosf(mFacingAngle));
	float idleX = -5.0f*scale*cosf(rotation)
		-10.0f*scale*cosf(mRotation+idleFrame->angles[BODY])
		+8.0f*scale*cosf(rotation+idleFrame->angles[RIGHTARM])
		+10.0f*scale*cosf(rotation+idleFrame->angles[RIGHTHAND])
		+scale*(mGuns[mGunIndex]->mGun->mSpriteOffsetX*cosf(mFacingAngle)-mGuns[mGunIndex]->mGun->mSpriteOffsetY*sinf(mFacingAngle));
	float idleY = -5.0f*scale*sinf(rotation)
		-10.0f*scale*sinf(mRotation+idleFrame->angles[BODY])
		+8.0f*scale*sinf(rotation+idleFrame->angles[RIGHTARM])
		+10.0f*scale*sinf(rotation+idleFrame->angles[RIGHTHAND])
		+scale*(mGuns[mGunIndex]->mGun->mSpriteOffsetX*sinf(mFacingAngle)+mGuns[mGunIndex]->mGun->mSpriteOffsetY*cosf(mFacingAngle));
	float angleDelta = mKeyFrame.angles[GUN]-idleFrame->angles[GUN];
	float pointFromIdleX = offsetX-idleX;
	float pointFromIdleY = offsetY-idleY;
	offsetX = currentX+pointFromIdleX*cosf(angleDelta)-pointFromIdleY*sinf(angleDelta);
	offsetY = currentY+pointFromIdleX*sinf(angleDelta)+pointFromIdleY*cosf(angleDelta);
	angle += angleDelta;
}

//------------------------------------------------------------------------------------------------
void Person::SetTotalRotation(float theta)
{
	mFacingAngle = theta;
	mLastFireAngle = theta-M_PI_2;
	mRotation = theta-M_PI_2;
	mAngle = theta;	
	mWalkAngle = theta;
}

//------------------------------------------------------------------------------------------------
void Person::Die()
{
	mStateTime = 0.0f;
	StopFire();
	Drop(mGunIndex,0);
	Drop(GRENADE,0);

	if (mSoundId != -1) {
		mSoundSystem->StopSample(mSoundId);
		mSoundId = -1;
	}
	if (mGuns[PRIMARY] != NULL) {
		delete mGuns[PRIMARY];
		mGuns[PRIMARY] = NULL;
	}
	if (mGuns[SECONDARY] != NULL) {
		delete mGuns[SECONDARY];
		mGuns[SECONDARY] = NULL;
	}
	/*if (mGunIndex != KNIFE) {
		mGuns[mGunIndex] = NULL;
	}*/
	if (mGuns[GRENADE] != NULL) {
		delete mGuns[GRENADE];
		mGuns[GRENADE] = NULL;
	}
	mGunIndex = KNIFE;
	mArmor = 0;

	mNumDryFire = 0;

	gSfxManager->PlaySample(gDieSounds[(mTeam == T) ? T : CT][rand()%3],mX,mY);
	SetState(DEAD);
	mFadeTime = 1000.0f;
	SetMoveState(NOTMOVING);
	mSpeed = 0.0f;
}

//------------------------------------------------------------------------------------------------
void Person::RestoreOriginalTeam()
{
	if (mOriginalTeam == NONE) {
		return;
	}
	mTeam = mOriginalTeam;
}

void Person::Reset()
{
	mMovementSpeedMultiplier = 1.0f;
	//mX = mSpawn->x;
	//mY = mSpawn->y;
	//SetPosition(mSpawn->x,mSpawn->y);
	//mOldX = mSpawn->x;
	//mOldY = mSpawn->y;
	SetTotalRotation(M_PI_2);
	SetMoveState(NOTMOVING);
	mSpeed = 0.0f;
	mFadeTime = 1000.0f;
	mInvincibleTime = 0.0f;
	mHasCorpse = false;
	mHealth = mTeam == T ? gPlayerConfig.tSpawnHealth : gPlayerConfig.ctSpawnHealth;
	mArmor = mTeam == T ? gPlayerConfig.tSpawnArmor : gPlayerConfig.ctSpawnArmor;
	if (mTeam == T || mTeam == CT) {
		SetKnifeForTeam(mTeam);
	}
	if (mState == DEAD) {
		SetState(NORMAL);
	}
	if (mState == ATTACKING || mState == DRYFIRING) {
		SetState(NORMAL);
	}
	mIsActive = false;
	mIsFlashed = false;

	mIsFiring = false;
	mHasFired = false;

	mWalkState = WALK1;
	mWalkTime = 0.0f;
	mWalkAngle = 0.0f;

	mMuzzleFlashTime = 0.0f;

	mRadarTime = 0.0f;

	mHasFlag = false;
}

void Person::TakeDamage(int damage) {
	if (mInvincibleTime > 0.0f) return;
	if (mState != DEAD) {
		SetMoveState(NOTMOVING);
		mSpeed *= gPlayerConfig.damageSpeedMultiplier;
		if (mArmor > 0) {
			int absorbedDamage = damage*GetArmorDamageReduction()/100;
			if (absorbedDamage > mArmor) absorbedDamage = mArmor;
			mArmor -= absorbedDamage;
			damage -= absorbedDamage;
		}
		mHealth -= damage;
		if (mHealth <= 0) {
			mHealth = 0;
			Die();
		}
	}
}

void Person::ReceiveFlash(float intensity) {
	if (mInvincibleTime > 0.0f) return;
	mIsFlashed = true;
	mFlashTime = gGrenadeConfig.flashDuration;
	if (intensity < 0.01f) {
		intensity = 0.01f;
	}
	mFlashIntensity = intensity;
}

void Person::SetQuads(JQuad* quads[], JQuad* deadquad) {
	for (int i=0; i<NUM_QUADS; i++) {
		mQuads[i] = quads[i];
		mOriginalQuads[i] = quads[i];
	}
	mDeadQuad = deadquad;
	mOriginalDeadQuad = deadquad;
}

void Person::LeaveCorpse() {
	mCorpseQuad = mDeadQuad;
	mCorpseX = mX;
	mCorpseY = mY;
	mCorpseRotation = mRotation;
	mHasCorpse = true;
}

void Person::SetTeamAppearance(int team) {
	int skin = GetDefaultPlayerSkin(team);
	for (int i=0; i<NUM_QUADS; i++) {
		mQuads[i] = gPlayersQuads[skin][i];
	}
	mDeadQuad = gPlayersDeadQuads[skin];
}

void Person::RestoreOriginalAppearance() {
	for (int i=0; i<NUM_QUADS; i++) {
		mQuads[i] = mOriginalQuads[i];
	}
	mDeadQuad = mOriginalDeadQuad;
}

void Person::UpdateAngle(float &angle, float targetangle, float speed) {
	float diffangle = angle-targetangle;
	if (diffangle > M_PI) {
		diffangle -= 2*M_PI;
	}
	if (diffangle < -M_PI) {
		diffangle += 2*M_PI;
	}
	if (diffangle >= 0) {
		if (diffangle < speed) {
			angle = targetangle;
		}
		else {
			angle -= speed;
		}
	}
	else {
		if (diffangle > -speed) {
			angle = targetangle;
		}
		else {
			angle += speed;
		}
	}
}

void Person::Teleport(float x, float y) {
	mX = x;
	mY = y;
	mOldX = x;
	mOldY = y;
	mWalkX = x;
	mWalkY = y;
}