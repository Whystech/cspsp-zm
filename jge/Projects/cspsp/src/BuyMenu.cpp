
#include "BuyMenu.h"
#include "Globals.h"

JGE* BuyMenu::mEngine = NULL;
JRenderer* BuyMenu::mRenderer = NULL;

//------------------------------------------------------------------------------------------------
BuyMenu::BuyMenu(Person* player, Gun guns[])
{
	for (int i=0; i<MAX_GUNS; i++) {
		mGuns[i] = guns[i];
	}

	mEngine = JGE::GetInstance();
	mRenderer = JRenderer::GetInstance();
	
	mPlayer = player;

	mChoice = -3;

	mIsActive = false;
	mIsSelected = false;
	mCategoryIndex = MAIN;
	mSelectedIndex = -1;
	mScrollOffset = 0;

	mCategories[T][MAIN].id = MAIN;
	mCategories[T][MAIN].buttons.push_back(Button(PISTOLS,"Pistols"));
	mCategories[T][MAIN].buttons.push_back(Button(SHOTGUNS,"Shotguns"));
	mCategories[T][MAIN].buttons.push_back(Button(SMG,"SMGs"));
	mCategories[T][MAIN].buttons.push_back(Button(RIFLES,"Rifles"));
	mCategories[T][MAIN].buttons.push_back(Button(MACHINEGUNS,"Machine Guns"));
	mCategories[T][MAIN].buttons.push_back(Button(EQUIPMENT,"Equipment"));
	mCategories[CT][MAIN] = mCategories[T][MAIN];

	for (int i=0; i<MAX_GUNS; i++) {
		if (guns[i].mId != i || guns[i].mName[0] == '\0' ||
			guns[i].mBuyCategory < PISTOLS || guns[i].mBuyCategory > EQUIPMENT) continue;
		if ((guns[i].mBuyTeams & BUY_TEAM_T) != 0) {
			mCategories[T][guns[i].mBuyCategory].buttons.push_back(Button(i,guns[i].mName));
		}
		if ((guns[i].mBuyTeams & BUY_TEAM_CT) != 0) {
			mCategories[CT][guns[i].mBuyCategory].buttons.push_back(Button(i,guns[i].mName));
		}
	}
	if (!mPlayer->mIsPlayerOnline) {
		mCategories[T][EQUIPMENT].buttons.push_back(Button(BUY_ITEM_ARMOR,"Armor"));
		mCategories[T][EQUIPMENT].buttons.push_back(Button(BUY_ITEM_HEALTH,"Health"));
		mCategories[CT][EQUIPMENT].buttons.push_back(Button(BUY_ITEM_ARMOR,"Armor"));
		mCategories[CT][EQUIPMENT].buttons.push_back(Button(BUY_ITEM_HEALTH,"Health"));
	}

	mIsOldStyle = true;
}

void BuyMenu::RenderItemIcon(int id, float x, float y, float scale)
{
	if (id == BUY_ITEM_ARMOR) {
		mRenderer->RenderQuad(gArmorGroundQuad,x,y,0,scale,scale);
	}
	else if (id == BUY_ITEM_HEALTH) {
		mRenderer->RenderQuad(gHealthGroundQuad,x,y,0,scale,scale);
	}
	else if (id >= 0 && id < MAX_GUNS && mGuns[id].mGroundQuad != NULL) {
		mRenderer->RenderQuad(mGuns[id].mGroundQuad,x,y,0,scale,scale);
	}
}

//------------------------------------------------------------------------------------------------
BuyMenu::~BuyMenu()
{
}

//------------------------------------------------------------------------------------------------
void BuyMenu::Update(float dt)
{
	int team = mPlayer->mTeam;
	if (mEngine->GetButtonClick(PSP_CTRL_CIRCLE)) {
		Disable();
		//mIsActive = false;
		return;
	}
	if (mEngine->GetButtonClick(PSP_CTRL_TRIANGLE) && mCategoryIndex != MAIN) {
		mCategoryIndex = MAIN;
		mScrollOffset = 0;
		mSelectedIndex = mIsOldStyle ? 0 : -1;
	}

	if (mEngine->GetButtonClick(PSP_CTRL_CROSS)) {
		if (mSelectedIndex != -1) {
			if (mCategoryIndex == MAIN) {
				mCategoryIndex = mCategories[team][mCategoryIndex].buttons[mSelectedIndex].id;
				mScrollOffset = 0;
				if (mIsOldStyle) {
					mSelectedIndex = 0;
				}
			}
			else {	
				mChoice = mCategories[team][mCategoryIndex].buttons[mSelectedIndex].id;
				mIsSelected = true;
				Disable();
			}
		}
		else {
			if (mCategoryIndex == MAIN) {
				mChoice = -1;
				mIsSelected = true;
				Disable();
			}
		}
	}

	if (!mIsOldStyle) {
		float aX = mEngine->GetAnalogX()-127.5f;
		float aY = mEngine->GetAnalogY()-127.5f;
		
		int size = mCategories[team][mCategoryIndex].buttons.size();
		if (aX >= 20 || aX <= -20 || aY >= 20 || aY <= -20) {
			angle = atan2f(aX,aY) + M_PI;
			mSelectedIndex = (int)(size*angle/(2*M_PI)+0.5f);
			if (mSelectedIndex == size) {
				mSelectedIndex = 0;
			}
		}
		else {
			mSelectedIndex = -1;
		}
	}
	else {
		int size = mCategories[team][mCategoryIndex].buttons.size();
		if (mEngine->GetButtonClick(PSP_CTRL_UP)) {
			mSelectedIndex--;
			if (mCategoryIndex == MAIN) {
				if (mSelectedIndex < -1) mSelectedIndex = size-1;
			}
			else {
				if (mSelectedIndex < 0) mSelectedIndex = size-1;
			}
		}
		else if (mEngine->GetButtonClick(PSP_CTRL_DOWN)) {
			mSelectedIndex++;
			if (mCategoryIndex == MAIN) {
				if (mSelectedIndex >= size) mSelectedIndex = -1;
			}
			else {
				if (mSelectedIndex >= size) mSelectedIndex = 0;
			}
		}

		const int visibleItems = 7;
		if (mCategoryIndex != MAIN) {
			if (mSelectedIndex < mScrollOffset) {
				mScrollOffset = mSelectedIndex;
			}
			else if (mSelectedIndex >= mScrollOffset+visibleItems) {
				mScrollOffset = mSelectedIndex-visibleItems+1;
			}
		}
	}
}



//------------------------------------------------------------------------------------------------
void BuyMenu::Render()
{	
	if (!mIsActive) return;
	// menu select color backgroun
	mRenderer->FillRect(300,25,SCREEN_WIDTH-300-10,SCREEN_HEIGHT-40,ARGB(220,0,0,0));
	mRenderer->DrawRect(300,25,SCREEN_WIDTH-300-10,SCREEN_HEIGHT-40,ARGB(255,255,128,0));
	//mRenderer->FillCircle(SCREEN_WIDTH_2,SCREEN_HEIGHT_2,SCREEN_HEIGHT_2,ARGB(200,50,50,50));
	gFont->SetColor(ARGB(255,255,255,255));
	gFont->SetScale(1.0f);
	gFont->DrawString("Buy Menu", 310, 40);
	gFont->SetScale(0.75f);

	if (!mIsOldStyle) {
		gFont->DrawString("[ANALOG+X] Select",320,70);
	}
	else {
		gFont->DrawString("[DIR PAD+X] Select",320,70);
	}
	gFont->DrawString("[O] Cancel",320,90);
	if (mCategoryIndex != MAIN) {
		gFont->DrawString("[^] Return",320,110);
	}

	int team = mPlayer->mTeam;
	int size = mCategories[team][mCategoryIndex].buttons.size();
	float theta = -M_PI_2;
	float step = size > 0 ? 1.0f/size*(2*M_PI) : 0.0f;
	const int visibleItems = 7;
	int firstVisible = mIsOldStyle && mCategoryIndex != MAIN ? mScrollOffset : 0;
	int lastVisible = mIsOldStyle && mCategoryIndex != MAIN ? mScrollOffset+visibleItems : size;
	if (lastVisible > size) lastVisible = size;
	float radialCardSize = size > 10 ? 30.0f : 40.0f;
	float radialSelectedSize = size > 10 ? 44.0f : 60.0f;
	float radialIconScale = size > 10 ? 0.8f : 1.0f;
	float radialSelectedIconScale = size > 10 ? 1.1f : 1.4f;
	for (int i=0; i<size; i++) {
		//float theta = (float)i/size*(2*M_PI);
		float x = 140+100*cosf(theta);
		float y = SCREEN_HEIGHT_2+100*sinf(theta);

		if (mIsOldStyle) {
			if (i < firstVisible || i >= lastVisible) continue;
			x = 50;
			y = 25+32*(i-firstVisible);
		}

		if (i == mSelectedIndex) {
			gFont->SetScale(1.0f);
			int id = 0;
			if (mCategoryIndex != MAIN) {
				id = mCategories[team][mCategoryIndex].buttons[i].id;
				//gFont->DrawShadowedString(mCategories[team][mCategoryIndex].buttons[i].name,x,y,JGETEXT_CENTER);
			}
			else {
				int cid = mCategories[team][mCategoryIndex].buttons[i].id;
				id = mCategories[team][cid].buttons[0].id;
			}
			
			if (!mIsOldStyle) {
				mRenderer->FillRect(x-radialSelectedSize/2,y-radialSelectedSize/2,radialSelectedSize,radialSelectedSize,ARGB(220,0,0,0));
				mRenderer->DrawRect(x-radialSelectedSize/2,y-radialSelectedSize/2,radialSelectedSize,radialSelectedSize,ARGB(255,255,128,0));
				RenderItemIcon(id,x,y-5,radialSelectedIconScale);
				gFont->DrawShadowedString(mCategories[team][mCategoryIndex].buttons[i].name,x,y,JGETEXT_CENTER);
			}
			else {
				mRenderer->FillRect(x,y,200,25,ARGB(220,0,0,0));
				mRenderer->DrawRect(x,y,200,25,ARGB(255,255,128,0));
				RenderItemIcon(id,x+25,y+12,1.4f);
				gFont->DrawShadowedString(mCategories[team][mCategoryIndex].buttons[i].name,x+50,y+3);
			}
			//mRenderer->FillPolygon(x,y,150,3,M_PI-theta,ARGB(200,255,255,255));
			//mRenderer->FillRect(x-75,y-25,150,50,ARGB(200,255,255,255));
		}
		else {
			gFont->SetScale(0.75f);
			int id = 0;
			if (mCategoryIndex != MAIN) {
				id = mCategories[team][mCategoryIndex].buttons[i].id;
			}
			else {
				int cid = mCategories[team][mCategoryIndex].buttons[i].id;
				id = mCategories[team][cid].buttons[0].id;
			}

			if (!mIsOldStyle) {
				mRenderer->FillRect(x-radialCardSize/2,y-radialCardSize/2,radialCardSize,radialCardSize,ARGB(220,0,0,0));
				mRenderer->DrawRect(x-radialCardSize/2,y-radialCardSize/2,radialCardSize,radialCardSize,ARGB(255,255,128,0));
				RenderItemIcon(id,x,y-5,radialIconScale);
				if (size <= 10) {
					gFont->DrawShadowedString(mCategories[team][mCategoryIndex].buttons[i].name,x,y+3,JGETEXT_CENTER);
				}
			}
			else {
				mRenderer->FillRect(x,y,200,25,ARGB(220,0,0,0));
				mRenderer->DrawRect(x,y,200,25,ARGB(255,255,128,0));
				RenderItemIcon(id,x+25,y+12,1.0f);
				gFont->DrawShadowedString(mCategories[team][mCategoryIndex].buttons[i].name,x+50,y+6);
			}

			//mRenderer->FillPolygon(x,y,150,3,M_PI-theta,ARGB(200,50,50,50));
			//mRenderer->FillRect(x-75,y-25,150,50,ARGB(200,50,50,50));
		}
		//mRenderer->FillRect(x-50,y-10,100,20,ARGB(200,50,50,50));
		
		theta -= step;
	}

	if (mIsOldStyle && mCategoryIndex != MAIN) {
		gFont->SetScale(0.75f);
		if (firstVisible > 0) gFont->DrawString("^",255,25);
		if (lastVisible < size) gFont->DrawString("v",255,217);
	}

	if (mCategoryIndex == MAIN) {
		float x = 140;
		float y = SCREEN_HEIGHT_2;
		if (mIsOldStyle) {
			x = 50;
			y = 25+32*size;
		}
		if (!mIsOldStyle) {
			if (mSelectedIndex == -1) {
				gFont->SetScale(1.0f);
				mRenderer->FillRect(x-30,y-30,60,60,ARGB(220,0,0,0));
				mRenderer->DrawRect(x-30,y-30,60,60,ARGB(255,255,128,0));
				gFont->DrawShadowedString("Ammo",x,y,JGETEXT_CENTER);
			}
			else {
				gFont->SetScale(0.75f);
				mRenderer->FillRect(x-20,y-20,40,40,ARGB(220,0,0,0));
				mRenderer->DrawRect(x-20,y-20,40,40,ARGB(255,255,128,0));
				gFont->DrawShadowedString("Ammo",x,y+3,JGETEXT_CENTER);
			}
		}
		else {
			if (mSelectedIndex == -1) {
				gFont->SetScale(1.0f);
				mRenderer->FillRect(x,y,200,25,ARGB(220,0,0,0));
				mRenderer->DrawRect(x,y,200,25,ARGB(255,255,128,0));
				gFont->DrawShadowedString("Ammo",x+50,y+3);
			}
			else {
				gFont->SetScale(0.75f);
				mRenderer->FillRect(x,y,200,25,ARGB(220,0,0,0));
				mRenderer->DrawRect(x,y,200,25,ARGB(255,255,128,0));
				gFont->DrawShadowedString("Ammo",x+50,y+6);
			}
		}
	}

	gFont->SetScale(0.6f);
	if (mCategoryIndex != MAIN && mSelectedIndex != -1) {
		int id = mCategories[team][mCategoryIndex].buttons[mSelectedIndex].id;
		RenderItemIcon(id,330,155,1.4f);

		float x = 310;
		float y = 175;
		if (id == BUY_ITEM_ARMOR) {
			char reduction[8];
			sprintf(reduction,"%i%%",GetArmorDamageReduction());
			gFont->DrawString("Damage reduction:",x,y);
			gFont->DrawString(reduction,x+110,y);
			y += 12;
			gFont->DrawString("Price",x,y);
			gFont->DrawString("$650",x+60,y);
			return;
		}
		if (id == BUY_ITEM_HEALTH) {
			gFont->DrawString("Restores health:",x,y);
			gFont->DrawString("Full",x+100,y);
			y += 12;
			gFont->DrawString("Price",x,y);
			gFont->DrawString("$500",x+60,y);
			return;
		}
		int width = 0;
		float value = 0.0f;
		float min = 0.0f;
		float max = 1.0f;

		if (id != 25 && id != 27) {
			gFont->DrawString("Damage:",x,y);
			mRenderer->FillRect(x+60,y+2,90,5,ARGB(255,100,100,100));

			value = mGuns[id].mDamage;
			min = 5;
			max = 40;
			if (mGuns[id].mPellets > 1) {
				value *= mGuns[id].mPellets;
				max = 100;
			}
			else if (mGuns[id].mScope >= SCOPE_MEDIUM) {
				max = 100;
			}
			else if (id >= 25 && id <= 27) { //nades
				max = 100;
			}
			width = 90*((value-min)/(max-min));
			if (width > 90) width = 90;
			else if (width < 0) width = 0;
			mRenderer->FillRect(x+60,y+2,width,5,ARGB(255,255,255,255));
			y += 12;
		}

		if (id != 25 && id != 26 && id != 27) {
			gFont->DrawString("Accuracy:",x,y);
			mRenderer->FillRect(x+60,y+2,90,5,ARGB(255,100,100,100));

			value = M_PI-mGuns[id].mSpread;
			min = M_PI_2+M_PI_4/2;
			max = M_PI;
			if (mGuns[id].mScope >= SCOPE_MEDIUM) {
				min = 0;
			}
			else if (id >= 25 && id <= 27) { //nades
				value = 0;
			}

			width = 90*((value-min)/(max-min));
			if (width > 90) width = 90;
			else if (width < 0) width = 0;
			mRenderer->FillRect(x+60,y+2,width,5,ARGB(255,255,255,255));
			y += 12;

			gFont->DrawString("Fire Rate:",x,y);
			mRenderer->FillRect(x+60,y+2,90,5,ARGB(255,100,100,100));

			value = 2000-mGuns[id].mDelay;
			min = 1850;
			max = 1950;
			if (mGuns[id].mPellets > 1) {
				min = 500;
				max = 3000;
			}
			else if (mGuns[id].mScope >= SCOPE_MEDIUM) {
				min = 250;
				max = 3500;
			}
			else if (id >= 25 && id <= 27) { //nades
				value = 0;
			}
			width = 90*((value-min)/(max-min));
			if (width > 90) width = 90;
			else if (width < 0) width = 0;
			mRenderer->FillRect(x+60,y+2,width,5,ARGB(255,255,255,255));
			y += 12;
		}


		char buffer[16];
		sprintf(buffer,"$%i",mGuns[id].mCost);
		gFont->DrawString("Price",x,y);
		gFont->DrawString(buffer,x+60,y);
		
		

	}

	//gFont->printf(SCREEN_WIDTH_2,SCREEN_HEIGHT_2,"%f",angle);
	/*if (mMainController->IsActive()) {
		mMainController->Render();
	}
	else {
		gFont->DrawString("[^] Return", 220, 90);
		for (int i=PISTOLS; i<=EQUIPMENT; i++) {
			if (mControllers[i]->IsActive()) {
				mControllers[i]->Render();
			}
		}
	}*/
	
	/*for (int i=PISTOLS; i<=EQUIPMENT; i++) {
		if (mControllers[i]->IsActive()) {
			int id = mControllers[i]->GetGuiObject(mControllers[i]->GetCurr())->GetId();
			mRenderer->RenderQuad(mGuns[id].mGroundQuad,236,126);
			char buffer[16];
			sprintf(buffer,"Price: $%i",mGuns[id].mCost);
			gFont->DrawString(buffer,220,142);
		}
	}*/
}

//------------------------------------------------------------------------------------------------
void BuyMenu::Enable()
{	
	mIsSelected = false;
	mIsActive = true;
	mCategoryIndex = MAIN;
	mScrollOffset = 0;
	if (!mIsOldStyle) {
		mSelectedIndex = -1;
	}
	else {
		mSelectedIndex = 0;
	}
	/*mMainController->SetCurr(0);
	mMainController->SetActive(true);
	mMainController->mLastKey = PSP_CTRL_UP;
	mMainController->mKeyRepeatDelay = JGUI_INITIAL_DELAY;
	for (int i=PISTOLS; i<=EQUIPMENT; i++) {
		mControllers[i]->SetCurr(0);
		mControllers[i]->SetActive(false);
	}*/
}

//------------------------------------------------------------------------------------------------
void BuyMenu::Disable()
{	
	//mIsSelected = false;
	mIsActive = false;
	*cross = true;
	//mMainController->SetCurr(0);
	//mMainController->SetActive(true);
	//mMainController->mLastKey = PSP_CTRL_UP;
	//mMainController->mKeyRepeatDelay = JGUI_INITIAL_DELAY;
	/*for (int i=PISTOLS; i<=MACHINEGUNS; i++) {
		mControllers[i]->SetCurr(0);
		mControllers[i]->SetActive(false);
	}*/
}

//------------------------------------------------------------------------------------------------
int BuyMenu::GetChoice()
{
	return mChoice;
}

/*//------------------------------------------------------------------------------------------------
void BuyMenu::ButtonPressed(int controllerId, int controlId)
{
	if (controllerId == MAIN) {
		if (controlId >= PISTOLS && controlId <= EQUIPMENT) {
			mMainController->SetActive(false);
			mControllers[controlId]->SetActive(true);
		}
		else if (controlId == PRIMARYAMMO) {
			mChoice = -1;
			mIsSelected = true;
			Disable();
		}
		else if (controlId == SECONDARYAMMO) {
			mChoice = -2;
			mIsSelected = true;
			Disable();
		}
	}
	else if (controllerId >= PISTOLS && controllerId <= EQUIPMENT) {
		mChoice = controlId;
		mIsSelected = true;
		Disable();
	}

}*/
