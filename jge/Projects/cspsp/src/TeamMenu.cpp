
#include "TeamMenu.h"
#include "Globals.h"

JGE* TeamMenu::mEngine = NULL;
JRenderer* TeamMenu::mRenderer = NULL;

#define TEAM_MENU_VISIBLE_ROWS 7

//------------------------------------------------------------------------------------------------
TeamMenu::TeamMenu()
{
	mEngine = JGE::GetInstance();
	mRenderer = JRenderer::GetInstance();
	
	mTeam = NONE;
	mType = 0;

	mIsActive = false;
	mIsSelected = false;
	mCategoryIndex = MAIN1;
	mSelectedIndex = -1;

	mCategories[MAIN1].id = MAIN1;
	if (gTeamSkinCounts[T] > 0) mCategories[MAIN1].buttons.push_back(Button(T,gTeamNames[T]));
	if (gTeamSkinCounts[CT] > 0) mCategories[MAIN1].buttons.push_back(Button(CT,gTeamNames[CT]));
	mCategories[MAIN1].buttons.push_back(Button(NONE,"Spectator"));

	mCategories[CT].id = CT;
	for (int i=0; i<gTeamSkinCounts[CT]; i++) {
		int id = gTeamSkinIds[CT][i];
		mCategories[CT].buttons.push_back(Button(id,gPlayerSkinNames[id]));
	}

	mCategories[T].id = T;
	for (int i=0; i<gTeamSkinCounts[T]; i++) {
		int id = gTeamSkinIds[T][i];
		mCategories[T].buttons.push_back(Button(id,gPlayerSkinNames[id]));
	}

	int ctSkin = GetDefaultPlayerSkin(CT);
	int tSkin = GetDefaultPlayerSkin(T);
	mCT = new Person(gPlayersQuads[ctSkin],gPlayersDeadQuads[ctSkin],NULL,NULL,CT,"test",0);
	mT = new Person(gPlayersQuads[tSkin],gPlayersDeadQuads[tSkin],NULL,NULL,T,"test",0);
	mCT->mGuns[KNIFE] = new GunObject(&gGuns[0],0,0);
	mT->mGuns[KNIFE] = new GunObject(&gGuns[ZOMBIECLAWS],0,0);
	mCT->PickUp(new GunObject(&gGuns[17],0,0));
	mT->PickUp(new GunObject(&gGuns[18],0,0));
	mCT->SetState(NORMAL);
	mT->SetState(NORMAL);
	mCT->mRenderScale = 1.6f;
	mT->mRenderScale = 1.6f;
	mCT->mX = -SCREEN_WIDTH_2+385;
	mCT->mY = -SCREEN_HEIGHT_2+205;
	mT->mX = -SCREEN_WIDTH_2+385;
	mT->mY = -SCREEN_HEIGHT_2+205;

	mIsOldStyle = true;

	FormatText(mTeamLines,"Zombies are infecting the last survivors. Neutralize the threat before humanity is wiped out.",150,0.6f);
	FormatText(mExterminationLines,"Eliminate the opposing team. A round ends when one team has no survivors.",150,0.6f);
	FormatText(mHordeLines,"Survive each Zombie wave. Eliminated UN Forces return only when the run restarts.",150,0.6f);
	FormatText(mCTFLines,"Capture the enemy flag while defending your own. The team with the most captures by the end of the round wins.",150,0.6f);
	FormatText(mFFALines,"Eliminate everyone else. The player with the most kills by the end of the round wins.\n\nTeam selection only matters for your appearance and weapons.",150,0.6f);
}

//------------------------------------------------------------------------------------------------
TeamMenu::~TeamMenu()
{
	
}

//------------------------------------------------------------------------------------------------
void TeamMenu::Update(float dt)
{
	if (mEngine->GetButtonClick(PSP_CTRL_CIRCLE)) {
		Disable();
		//mIsActive = false;
		//mTeam = CT;
		return;
	}
	if (mEngine->GetButtonClick(PSP_CTRL_TRIANGLE) && mCategoryIndex != MAIN1) {
		mCategoryIndex = MAIN1;
	}

	if (mEngine->GetButtonClick(PSP_CTRL_CROSS)) {
		if (mSelectedIndex != -1) {
			int id = mCategories[mCategoryIndex].buttons[mSelectedIndex].id;
			if (mCategoryIndex == MAIN1 && id != NONE) {
				mCategoryIndex = id;
				if (mIsOldStyle) {
					mSelectedIndex = 0;
				}
			}
			else if (mCategoryIndex == MAIN1 && id == NONE) {
				mTeam = NONE;
				mType = 0;
				mIsSelected = true;
				Disable();
			}
			else {	
				mTeam = mCategoryIndex;
				mType = id;
				mIsSelected = true;
				Disable();
			}
		}
	}

	int size = mCategories[mCategoryIndex].buttons.size();
	bool useList = mIsOldStyle || (mCategoryIndex != MAIN1 && size > 8);
	if (!useList) {
		float aX = mEngine->GetAnalogX()-127.5f;
		float aY = mEngine->GetAnalogY()-127.5f;
		
		if (aX >= 20 || aX <= -20 || aY >= 20 || aY <= -20) {
			angle = atan2f(aX,aY) + M_PI;
			if (mCategoryIndex == MAIN1) {
				angle += M_PI/3;
			}
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
		if (mEngine->GetButtonClick(PSP_CTRL_UP)) {
			mSelectedIndex--;
			if (mSelectedIndex < 0) mSelectedIndex = size-1;
		}
		else if (mEngine->GetButtonClick(PSP_CTRL_DOWN)) {
			mSelectedIndex++;
			if (mSelectedIndex >= size) mSelectedIndex = 0;
		}
	}

	mCT->Update(dt);
	mT->Update(dt);
}



//------------------------------------------------------------------------------------------------
void TeamMenu::Render()
{	
	if (!mIsActive) return;
	mRenderer->FillRect(300,25,SCREEN_WIDTH-300-10,SCREEN_HEIGHT-40,ARGB(220,0,0,0));
	mRenderer->DrawRect(300,25,SCREEN_WIDTH-300-10,SCREEN_HEIGHT-40,ARGB(255,255,128,0));
	//mRenderer->FillCircle(SCREEN_WIDTH_2,SCREEN_HEIGHT_2,SCREEN_HEIGHT_2,ARGB(200,50,50,50));
	gFont->SetColor(ARGB(255,255,255,255));
	gFont->SetScale(1.0f);
	gFont->DrawString("Team Select", 310, 40);
	gFont->SetScale(0.75f);

	int size = mCategories[mCategoryIndex].buttons.size();
	bool useList = mIsOldStyle || (mCategoryIndex != MAIN1 && size > 8);
	if (!useList) {
		gFont->DrawString("[ANALOG+X] Select",320,70);
	}
	else {
		gFont->DrawString("[DIR PAD+X] Select",320,70);
	}
	gFont->DrawString("[O] Cancel",320,90);
	if (mCategoryIndex != MAIN1) {
		gFont->DrawString("[^] Return",320,110);
	}

	if (mCategoryIndex == MAIN1) {
		if (*mGameType == TEAM || *mGameType == EXTERMINATION || *mGameType == INFECTION || *mGameType == HORDE) {
			gFont->SetScale(0.75f);
			const char* modeName = "Infection";
			if (*mGameType == EXTERMINATION) modeName = "Extermination";
			else if (*mGameType == INFECTION) modeName = "Infection";
			else if (*mGameType == HORDE) modeName = "Horde";
			gFont->DrawString(modeName,310,140);
			gFont->SetScale(0.6f);
			std::vector<char*>* lines = &mExterminationLines;
			if (*mGameType == TEAM || *mGameType == INFECTION) lines = &mTeamLines;
			else if (*mGameType == HORDE) lines = &mHordeLines;
			for (unsigned int i=0; i<lines->size(); i++) {
				gFont->DrawString((*lines)[i],315,160+10*i);
			}
		}
		else if (*mGameType == FFA) {
			gFont->SetScale(0.75f);
			gFont->DrawString("Free for All",310,140);
			gFont->SetScale(0.6f);
			for (int i=0; i<mFFALines.size(); i++) {
				gFont->DrawString(mFFALines[i],315,160+10*i);
			}
		}
		else if (*mGameType == CTF) {
			gFont->SetScale(0.75f);
			gFont->DrawString("Capture the Flag",310,140);
			gFont->SetScale(0.6f);
			for (int i=0; i<mCTFLines.size(); i++) {
				gFont->DrawString(mCTFLines[i],315,160+10*i);
			}
		}
	}
	else {
		gFont->SetScale(0.75f);
		gFont->DrawString(gTeamNames[mCategoryIndex],310,140);
		if (size > TEAM_MENU_VISIBLE_ROWS) {
			char pageText[32];
			sprintf(pageText,"Skin %d / %d",mSelectedIndex+1,size);
			gFont->SetScale(0.6f);
			gFont->DrawString(pageText,310,160);
		}
	}

	float theta = -M_PI_2;
	if (mCategoryIndex == MAIN1) {
		theta += M_PI/3;
	}
	float step = 1.0f/size*(2*M_PI);
	int first = 0;
	int last = size;
	if (useList && size > TEAM_MENU_VISIBLE_ROWS) {
		first = mSelectedIndex-TEAM_MENU_VISIBLE_ROWS/2;
		if (first < 0) first = 0;
		if (first > size-TEAM_MENU_VISIBLE_ROWS) first = size-TEAM_MENU_VISIBLE_ROWS;
		last = first+TEAM_MENU_VISIBLE_ROWS;
	}
	for (int i=first; i<last; i++) {
		//float theta = (float)i/size*(2*M_PI);
		float x = 140+75*cosf(theta);
		float y = SCREEN_HEIGHT_2+75*sinf(theta);
		if (useList) {
			x = 50;
			y = 25+32*(i-first);
		}

		if (i == mSelectedIndex) {
			gFont->SetColor(ARGB(255,255,255,255));
			if (mCategoryIndex == MAIN1) {
				if (i == T) {
					gFont->SetColor(ARGB(255,255,64,64));
				}
				else if (i == CT) {
					gFont->SetColor(ARGB(255,153,204,255));
				}
			}
			else if (mCategoryIndex == T) {
				gFont->SetColor(ARGB(255,255,64,64));
			}
			else if (mCategoryIndex == CT) {
				gFont->SetColor(ARGB(255,153,204,255));
			}

			gFont->SetScale(1.0f);
			if (!useList) {
				mRenderer->FillRect(x-30,y-30,60,60,ARGB(220,0,0,0));
				mRenderer->DrawRect(x-30,y-30,60,60,ARGB(255,255,128,0));
				gFont->DrawShadowedString(mCategories[mCategoryIndex].buttons[i].name,x,y-8,JGETEXT_CENTER);
			}
			else {
				mRenderer->FillRect(x,y,200,25,ARGB(220,0,0,0));
				mRenderer->DrawRect(x,y,200,25,ARGB(255,255,128,0));
				gFont->DrawShadowedString(mCategories[mCategoryIndex].buttons[i].name,x+10,y+3);
			}
			//mRenderer->FillPolygon(x,y,150,3,M_PI-theta,ARGB(200,255,255,255));
			//mRenderer->FillRect(x-75,y-25,150,50,ARGB(200,255,255,255));
		}
		else {
			gFont->SetScale(0.75f);

			gFont->SetColor(ARGB(255,255,255,255));
			if (mCategoryIndex == MAIN1) {
				if (i == T) {
					gFont->SetColor(ARGB(255,255,64,64));
				}
				else if (i == CT) {
					gFont->SetColor(ARGB(255,153,204,255));
				}
			}
			else if (mCategoryIndex == T) {
				gFont->SetColor(ARGB(255,255,64,64));
			}
			else if (mCategoryIndex == CT) {
				gFont->SetColor(ARGB(255,153,204,255));
			}

			if (!useList) {
				mRenderer->FillRect(x-20,y-20,40,40,ARGB(220,0,0,0));
				mRenderer->DrawRect(x-20,y-20,40,40,ARGB(255,255,128,0));
				gFont->DrawShadowedString(mCategories[mCategoryIndex].buttons[i].name,x,y-5,JGETEXT_CENTER);
			}
			else {
				mRenderer->FillRect(x,y,200,25,ARGB(220,0,0,0));
				mRenderer->DrawRect(x,y,200,25,ARGB(255,255,128,0));
				gFont->DrawShadowedString(mCategories[mCategoryIndex].buttons[i].name,x+10,y+6);
			}
			//mRenderer->FillPolygon(x,y,150,3,M_PI-theta,ARGB(200,50,50,50));
			//mRenderer->FillRect(x-75,y-25,150,50,ARGB(200,50,50,50));
		}
		//mRenderer->FillRect(x-50,y-10,100,20,ARGB(200,50,50,50));
		
		theta -= step;
	}
	if (mCategoryIndex != MAIN1 && mSelectedIndex != -1) {
		int id = mCategories[mCategoryIndex].buttons[mSelectedIndex].id;
		if (mCategoryIndex == CT) {
			mCT->SetQuads(gPlayersQuads[id],gPlayersDeadQuads[id]);
			mCT->Render(0,0);
		}
		else if (mCategoryIndex == T) {
			mT->SetQuads(gPlayersQuads[id],gPlayersDeadQuads[id]);
			mT->Render(0,0);
		}
	}

	//gFont->SetScale(0.6f);
}

//------------------------------------------------------------------------------------------------
void TeamMenu::Enable()
{	
	mIsSelected = false;
	mIsActive = true;
	mCategoryIndex = MAIN1;
	if (!mIsOldStyle) {
		mSelectedIndex = -1;
	}
	else {
		mSelectedIndex = 0;
	}
}

//------------------------------------------------------------------------------------------------
void TeamMenu::Disable()
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
int TeamMenu::GetTeam()
{
	return mTeam;
}

//------------------------------------------------------------------------------------------------
int TeamMenu::GetType()
{
	return mType;
}