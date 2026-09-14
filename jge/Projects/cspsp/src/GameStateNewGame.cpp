#include "GameStateNewGame.h"

#define STAGE_MODE_SETUP 0
#define STAGE_MAP_SELECTION 1
#define MODE_LOCATION_INPLACE 0
#define MODE_LOCATION_BASE 1
#define MODE_OPTION_OFF 0
#define MODE_OPTION_ON 1

GameStateNewGame::GameStateNewGame(GameApp* parent): GameState(parent)
{
}

GameStateNewGame::~GameStateNewGame()
{

}


void GameStateNewGame::Create()
{
	mMapsListBox = new ListBox(0,65,SCREEN_WIDTH,175,25,7);

	strcpy(mSearchString,"");
	mMatch = true;
	mMatchIndex = -1;

	gFont->SetBase(0);	// using 2nd font

	int y = 20;
	#ifdef WIN32
	for (int i=0; i<10; i++) {
		char buffer[10];
		sprintf(buffer,"test%d",i);
		mMapsListBox->AddItem(new MapItem(buffer));
	}
	#else
	
	int index = 0;
	DIR *dip;
	struct dirent *dit;
	dip = opendir("maps");
	char fullname[512];

	while ((dit = readdir(dip)) != NULL)
	{
		static char * name; 
		name = (char*)memalign( 16, 300); 
		sprintf(name,"%s" , dit->d_name);  
	 
		if ((FIO_S_IFREG & (dit->d_stat.st_mode & FIO_S_IFMT)) == 0 && stricmp (name, ".") != 0 && stricmp (name, "..") != 0) 
		{
			mMapsListBox->AddItem(new MapItem(name));
		}
	}
	closedir(dip);


	/**int index = 0;
	int fd; 
	struct SceIoDirent dirent; 
	fd = sceIoDopen("ms0:/psp/game/cspsp/maps"); 

	memset( &dirent, 0, sizeof(SceIoDirent)); 
	 
	while (sceIoDread(fd, &dirent) > 0) 
	{ 
		static char * name; 
		name = (char*)memalign( 16, 300); 
		sprintf(name,"%s" , dirent.d_name); 
		//sprintf(fullname, "%s/%s", "ms0:/PSP", dirent.d_name); 
	 
		if ((FIO_S_IFREG & (dirent.d_stat.st_mode & FIO_S_IFMT)) == 0 && stricmp (name, ".") != 0 && stricmp (name, "..") != 0) 
		{ 
			if (index == 0) {
				mGuiController->Add(new MenuItem(index, gFont, name, 20, y, TYPE_NORMAL, JGETEXT_LEFT, true));
			}else{
				mGuiController->Add(new MenuItem(index, gFont, name, 20, y, TYPE_NORMAL, JGETEXT_LEFT, false));
			}
			maps.push_back(name);
			index++;
			y += 20;
		} 
	} 
	sceIoDclose(fd);**/

	#endif

	mMapsListBox->Sort(MapItem::Compare);

	float x = 190.0f;
	mModeControllers[0] = new JGuiController(0,this,JGUI_STYLE_LEFTRIGHT);
	mModeControllers[0]->Add(new MenuItem(MODE_LOCATION_INPLACE,gFont,"In-Place",x,100,TYPE_OPTION,JGETEXT_LEFT));
	mModeControllers[0]->Add(new MenuItem(MODE_LOCATION_BASE,gFont,"T Base",x+110,100,TYPE_OPTION,JGETEXT_LEFT));
	mModeControllers[1] = new JGuiController(1,this,JGUI_STYLE_LEFTRIGHT);
	for (int i=0; i<=4; i++) {
		char label[2];
		sprintf(label,"%d",i);
		mModeControllers[1]->Add(new MenuItem(i,gFont,label,x+i*38,125,TYPE_OPTION,JGETEXT_LEFT));
	}
	mModeControllers[2] = new JGuiController(2,this,JGUI_STYLE_LEFTRIGHT);
	mModeControllers[2]->Add(new MenuItem(MODE_LOCATION_INPLACE,gFont,"In-Place",x,100,TYPE_OPTION,JGETEXT_LEFT));
	mModeControllers[2]->Add(new MenuItem(MODE_LOCATION_BASE,gFont,"CT Base",x+110,100,TYPE_OPTION,JGETEXT_LEFT));
	mModeControllers[3] = new JGuiController(3,this,JGUI_STYLE_LEFTRIGHT);
	const int delays[] = {3,5,10,15};
	for (int i=0; i<4; i++) {
		char label[3];
		sprintf(label,"%d",delays[i]);
		mModeControllers[3]->Add(new MenuItem(delays[i],gFont,label,x+i*48,125,TYPE_OPTION,JGETEXT_LEFT));
	}
	mModeControllers[4] = new JGuiController(4,this,JGUI_STYLE_LEFTRIGHT);
	mModeControllers[4]->Add(new MenuItem(MODE_OPTION_OFF,gFont,"Off",x,150,TYPE_OPTION,JGETEXT_LEFT));
	mModeControllers[4]->Add(new MenuItem(MODE_OPTION_ON,gFont,"On",x+70,150,TYPE_OPTION,JGETEXT_LEFT));
	for (int i=0; i<5; i++) mModeControllers[i]->SetActive(false);
}


void GameStateNewGame::Destroy()
{
	SAFE_DELETE(mMapsListBox);
	for (int i=0; i<5; i++) SAFE_DELETE(mModeControllers[i]);
}


void GameStateNewGame::Start()
{
	mRenderer->EnableVSync(true);
	mStage = (gSinglePlayerMode == SINGLEPLAYER_INFECTION || gSinglePlayerMode == SINGLEPLAYER_HORDE) ? STAGE_MODE_SETUP : STAGE_MAP_SELECTION;
	mSetupIndex = 0;
	mSetupMain = true;
	LoadModeSettings();
}


void GameStateNewGame::End()
{
	mRenderer->EnableVSync(false);
}


void GameStateNewGame::Update(float dt)
{
	if (mStage == STAGE_MODE_SETUP) {
		int firstController = gSinglePlayerMode == SINGLEPLAYER_INFECTION ? 0 : 2;
		int setupCount = gSinglePlayerMode == SINGLEPLAYER_INFECTION ? 3 : 4;
		if (mSetupMain) {
			if (mEngine->GetButtonClick(PSP_CTRL_UP) || mEngine->GetAnalogY()<64) {
				mSetupIndex--;
				if (mSetupIndex < 0) mSetupIndex = setupCount-1;
			}
			else if (mEngine->GetButtonClick(PSP_CTRL_DOWN) || mEngine->GetAnalogY()>192) {
				mSetupIndex++;
				if (mSetupIndex >= setupCount) mSetupIndex = 0;
			}
			if (mEngine->GetButtonClick(PSP_CTRL_CROSS)) {
				if (mSetupIndex == setupCount-1) {
					SaveModeSettings();
					mStage = STAGE_MAP_SELECTION;
				}
				else {
					mSetupMain = false;
					mModeControllers[firstController+mSetupIndex]->SetActive(true);
				}
			}
			if (mEngine->GetButtonClick(PSP_CTRL_CIRCLE)) mParent->SetNextState(GAME_STATE_MENU);
		}
		else {
			if (mEngine->GetButtonClick(PSP_CTRL_CIRCLE)) {
				mModeControllers[firstController+mSetupIndex]->SetActive(false);
				mSetupMain = true;
			}
			mModeControllers[firstController+mSetupIndex]->Update(dt);
		}
		return;
	}

	if (!gDanzeff->mIsActive) {
		if (mEngine->GetButtonClick(PSP_CTRL_CIRCLE)) {
			if (gSinglePlayerMode == SINGLEPLAYER_INFECTION || gSinglePlayerMode == SINGLEPLAYER_HORDE) mStage = STAGE_MODE_SETUP;
			else mParent->SetNextState(GAME_STATE_MENU);
		}
		if (mEngine->GetButtonClick(PSP_CTRL_SQUARE)) {
			gDanzeff->Enable();
			gDanzeff->mString = mSearchString;
		}
		if (mEngine->GetButtonClick(PSP_CTRL_CROSS)) {
			MapItem *item = (MapItem*)mMapsListBox->GetItem();
			if (item != NULL) {
				gMapName = item->name;
				mParent->SetNextState(GAME_STATE_PLAY);
				return;
			}
		}

		mMapsListBox->Update(dt);
	}
	else {
		gDanzeff->Update(dt);
		if (gDanzeff->mString.length() > 50) {
			gDanzeff->mString = gDanzeff->mString.substr(0,50);
		}
		bool search = false;
		if (stricmp(mSearchString,(char*)gDanzeff->mString.c_str()) != 0) {
			search = true;
			strcpy(mSearchString,(char*)gDanzeff->mString.c_str());
		}
		if (mEngine->GetButtonClick(PSP_CTRL_START)) {
			search = true;
			gDanzeff->Disable();
		}
		else if (mEngine->GetButtonClick(PSP_CTRL_SELECT)) {
			gDanzeff->Disable();
		}

		if (search) {
			mMatch = false;
			mMatchIndex = -1;
			if (strlen(mSearchString) > 0) {
				for (int i=0; i<mMapsListBox->mItems.size(); i++) {
					MapItem *item = (MapItem*)mMapsListBox->mItems[i];
					//printf("%d\n",strnicmp(mSearchString,mMaps[i].name,strlen(mSearchString)));
					int s = strnicmp(mSearchString,item->name,strlen(mSearchString));
					if (s == 0) {
						mMatch = true;
						mMatchIndex = i;
						break;
					}
					else if (s < 0) {
						mMatch = false;
						mMatchIndex = -1;
						break;
					}
				}
			}
			for (int i=0; i<mMapsListBox->mItems.size(); i++) {
				MapItem *item = (MapItem*)mMapsListBox->mItems[i];
				if (i == mMatchIndex) {
					item->match = true;
					mMapsListBox->SetIndices(mMatchIndex,mMatchIndex);
				}
				else {
					item->match = false;
				}
			}
		}
	}
}


void GameStateNewGame::Render()
{
	if (mStage == STAGE_MODE_SETUP) {
		RenderModeSetup();
		return;
	}
	mRenderer->ClearScreen(ARGB(255,255,255,255));
	//mRenderer->FillRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ARGB(255,255,255,255));
	mRenderer->RenderQuad(gBgQuad, 0.0f, 0.0f);
	//mEngine->FillRect(15,15,SCREEN_WIDTH-30,SCREEN_HEIGHT-30,ARGB(150,0,0,0));

	mRenderer->FillRect(0,65,SCREEN_WIDTH,175,ARGB(100,0,0,0));
	mRenderer->FillRect(0,35,SCREEN_WIDTH,30,ARGB(175,0,0,0));
	mRenderer->DrawLine(90,35,90,65,ARGB(255,255,255,255));

	gFont->SetColor(ARGB(255,255,255,255));


	gFont->SetScale(1.0f);
	gFont->DrawShadowedString("Map Selection",20,10);
	gFont->DrawShadowedString("search:",20,40);
	if (mMatch) {
		gFont->SetColor(ARGB(255,255,128,0));
	}
	else {
		gFont->SetColor(ARGB(255,255,0,0));
	}
	gFont->DrawShadowedString(mSearchString,100,40);

	gFont->SetColor(ARGB(255,255,255,255));
	if (gDanzeff->mIsActive) {
		gFont->DrawShadowedString("|",100+gFont->GetStringWidth(mSearchString),40);
	}

	mMapsListBox->Render();

	gFont->SetColor(ARGB(255,255,255,255));
	gFont->SetScale(0.75f);
	if (gDanzeff->mIsActive) {
		gFont->DrawShadowedString("[START] Enter    [SELECT] Cancel",SCREEN_WIDTH_2,SCREEN_HEIGHT_F-20,JGETEXT_CENTER);
		gDanzeff->Render(SCREEN_WIDTH-175,SCREEN_HEIGHT-175);
	}
	else {
		gFont->DrawShadowedString("[X] Select     [[]] Search     [O] Return to Menu",SCREEN_WIDTH_2,SCREEN_HEIGHT_F-20,JGETEXT_CENTER);
	}
}

void GameStateNewGame::LoadModeSettings()
{
	mRespawnStyle = MODE_LOCATION_BASE;
	char* respawn = GetConfig("data/config.txt","respawn");
	if (respawn != NULL) {
		if (strcmp(respawn,"inplace") == 0) mRespawnStyle = MODE_LOCATION_INPLACE;
		delete respawn;
	}
	mInfectionDelay = 1;
	char* infectionDelay = GetConfig("data/config.txt","infection_respawn_delay");
	if (infectionDelay != NULL) {
		mInfectionDelay = atoi(infectionDelay);
		if (mInfectionDelay < 0 || mInfectionDelay > 4) mInfectionDelay = 1;
		delete infectionDelay;
	}
	mHordeRegroup = MODE_LOCATION_BASE;
	char* regroup = GetConfig("data/config.txt","horde_regroup");
	if (regroup != NULL) {
		if (strcmp(regroup,"inplace") == 0) mHordeRegroup = MODE_LOCATION_INPLACE;
		delete regroup;
	}
	mHordeWaveDelay = 3;
	char* waveDelay = GetConfig("data/config.txt","horde_wave_delay");
	if (waveDelay != NULL) {
		int delay = atoi(waveDelay);
		if (delay == 3 || delay == 5 || delay == 10 || delay == 15) mHordeWaveDelay = delay;
		delete waveDelay;
	}
	mHordeReviveSurvivors = MODE_OPTION_OFF;
	char* reviveSurvivors = GetConfig("data/config.txt","horde_revive_survivors");
	if (reviveSurvivors == NULL) reviveSurvivors = GetConfig("data/modes.txt","horde_revive_survivors");
	if (reviveSurvivors != NULL) {
		if (strcmp(reviveSurvivors,"on") == 0) mHordeReviveSurvivors = MODE_OPTION_ON;
		delete reviveSurvivors;
	}
	mModeControllers[0]->SetCurr(mRespawnStyle);
	mModeControllers[1]->SetCurr(mInfectionDelay);
	mModeControllers[2]->SetCurr(mHordeRegroup);
	int hordeDelayIndex = 0;
	if (mHordeWaveDelay == 5) hordeDelayIndex = 1;
	else if (mHordeWaveDelay == 10) hordeDelayIndex = 2;
	else if (mHordeWaveDelay == 15) hordeDelayIndex = 3;
	mModeControllers[3]->SetCurr(hordeDelayIndex);
	mModeControllers[4]->SetCurr(mHordeReviveSurvivors);
}

void GameStateNewGame::SaveModeSettings()
{
	char value[16];
	if (gSinglePlayerMode == SINGLEPLAYER_INFECTION) {
		SetConfigValue("data/config.txt","respawn",mRespawnStyle == MODE_LOCATION_INPLACE ? "inplace" : "base");
		sprintf(value,"%d",mInfectionDelay);
		SetConfigValue("data/config.txt","infection_respawn_delay",value);
	}
	else if (gSinglePlayerMode == SINGLEPLAYER_HORDE) {
		SetConfigValue("data/config.txt","horde_regroup",mHordeRegroup == MODE_LOCATION_INPLACE ? "inplace" : "base");
		sprintf(value,"%d",mHordeWaveDelay);
		SetConfigValue("data/config.txt","horde_wave_delay",value);
		SetConfigValue("data/config.txt","horde_revive_survivors",mHordeReviveSurvivors == MODE_OPTION_ON ? "on" : "off");
	}
}

void GameStateNewGame::RenderModeSetup()
{
	mRenderer->ClearScreen(ARGB(255,255,255,255));
	mRenderer->RenderQuad(gBgQuad,0.0f,0.0f);
	mRenderer->FillRect(0,55,SCREEN_WIDTH,140,ARGB(150,0,0,0));
	gFont->SetColor(ARGB(255,255,255,255));
	gFont->SetScale(1.0f);
	gFont->DrawShadowedString(gSinglePlayerMode == SINGLEPLAYER_INFECTION ? "Infection Setup" : "Horde Setup",SCREEN_WIDTH_2,25,JGETEXT_CENTER);
	gFont->SetScale(0.75f);
	const char* firstLabel = gSinglePlayerMode == SINGLEPLAYER_INFECTION ? "Zombie Respawn" : "Survivor Regroup";
	const char* secondLabel = gSinglePlayerMode == SINGLEPLAYER_INFECTION ? "Respawn Delay" : "Wave Delay";
	int setupCount = gSinglePlayerMode == SINGLEPLAYER_INFECTION ? 3 : 4;
	const char* info[] = {
		gSinglePlayerMode == SINGLEPLAYER_INFECTION ? "Choose where converted players return." : "Choose where survivors begin the next wave.",
		gSinglePlayerMode == SINGLEPLAYER_INFECTION ? "Seconds before a converted player respawns." : "Seconds between Horde waves.",
		gSinglePlayerMode == SINGLEPLAYER_INFECTION ? "Continue to map selection." : "Revive eliminated survivors after a cleared wave.",
		"Continue to map selection."
	};
	if (mSetupMain) mRenderer->FillRect(0,95+mSetupIndex*25,SCREEN_WIDTH,20,ARGB(180,0,0,0));
	else mRenderer->FillRect(0,95+mSetupIndex*25,SCREEN_WIDTH,20,ARGB(100,255,128,0));
	gFont->DrawShadowedString(firstLabel,170,100,JGETEXT_RIGHT);
	gFont->DrawShadowedString(secondLabel,170,125,JGETEXT_RIGHT);
	if (gSinglePlayerMode == SINGLEPLAYER_HORDE) gFont->DrawShadowedString("Revive Survivors",170,150,JGETEXT_RIGHT);
	gFont->DrawShadowedString("Continue",170,100+(setupCount-1)*25,JGETEXT_RIGHT);
	int firstController = gSinglePlayerMode == SINGLEPLAYER_INFECTION ? 0 : 2;
	mModeControllers[firstController]->Render();
	mModeControllers[firstController+1]->Render();
	if (gSinglePlayerMode == SINGLEPLAYER_HORDE) mModeControllers[4]->Render();
	gFont->SetColor(ARGB(255,255,128,0));
	gFont->DrawShadowedString(info[mSetupIndex],SCREEN_WIDTH_2,200,JGETEXT_CENTER);
	gFont->SetColor(ARGB(255,255,255,255));
	gFont->DrawShadowedString(mSetupMain ? "[X] Select     [O] Return" : "[DIR PAD/ANALOG] Change     [X] Select     [O] Cancel",SCREEN_WIDTH_2,SCREEN_HEIGHT_F-20,JGETEXT_CENTER);
}

void GameStateNewGame::ButtonPressed(int controllerId, int controlId)
{
	mModeControllers[controllerId]->SetActive(false);
	mSetupMain = true;
	if (controllerId == 0) mRespawnStyle = controlId;
	else if (controllerId == 1) mInfectionDelay = controlId;
	else if (controllerId == 2) mHordeRegroup = controlId;
	else if (controllerId == 3) mHordeWaveDelay = controlId;
	else if (controllerId == 4) mHordeReviveSurvivors = controlId;
}