#include "GameStateLoading.h"

static JTexture* LoadConfiguredTexture(JRenderer* renderer, char* key, char* fallback, bool mipmap)
{
	char* configured = GetConfig("data/resources.txt",key);
	JTexture* texture = renderer->LoadTexture(configured == NULL ? fallback : configured,mipmap);
	delete[] configured;
	return texture;
}

static JTexture* LoadOptionalConfiguredTexture(JRenderer* renderer, char* key, bool mipmap)
{
	char* configured = GetConfig("data/resources.txt",key);
	if (configured == NULL) return NULL;
	FILE* file = fopen(configured,"rb");
	if (file == NULL) {
		delete[] configured;
		return NULL;
	}
	fclose(file);
	JTexture* texture = renderer->LoadTexture(configured,mipmap);
	delete[] configured;
	return texture;
}

static JSample* LoadConfiguredSample(JSoundSystem* soundSystem, char* key, char* fallback)
{
	char* configured = GetConfig("data/audio.txt",key);
	JSample* sample = soundSystem->LoadSample(configured == NULL ? fallback : configured);
	delete[] configured;
	return sample;
}

static JSample* LoadConfiguredSampleWithLegacyKey(JSoundSystem* soundSystem, char* key, char* legacyKey, char* fallback)
{
	char* configured = GetConfig("data/audio.txt",key);
	if (configured == NULL) configured = GetConfig("data/audio.txt",legacyKey);
	JSample* sample = soundSystem->LoadSample(configured == NULL ? fallback : configured);
	delete[] configured;
	return sample;
}

static char* SkipPlayerDataWhitespace(char* text)
{
	while (*text == ' ' || *text == '\t') text++;
	return text;
}

static void TrimPlayerDataText(char* text)
{
	int length = strlen(text);
	while (length > 0 && (text[length-1] == '\r' || text[length-1] == '\n' || text[length-1] == ' ' || text[length-1] == '\t')) {
		text[--length] = '\0';
	}
}

static bool AddPlayerSkin(int id, int team, int atlasId, char* name, JTexture* playersTextures[], JTexture* playersDeadTextures[])
{
	if (id < 0 || id >= MAX_PLAYER_SKINS || team < T || team > CT || atlasId < 0 || name == NULL || name[0] == '\0') return false;
	if (gTeamSkinCounts[team] >= MAX_PLAYER_SKINS/2) return false;
	int page = atlasId/8;
	int pageCell = atlasId%8;
	if (page < 0 || page >= 4 || playersTextures[page] == NULL || playersDeadTextures[page] == NULL) return false;
	JTexture* playersTexture = playersTextures[page];
	JTexture* playersDeadTexture = playersDeadTextures[page];
	if (gPlayerSkinLoaded[id] || pageCell*32+32 > playersTexture->mTexWidth || playersTexture->mTexHeight < 64) return false;
	if (pageCell*32+29 > playersDeadTexture->mTexWidth || playersDeadTexture->mTexHeight < 47) return false;

	int offsetX = pageCell*32;
	gPlayersQuads[id][BODY] = new JQuad(playersTexture,offsetX,0,32,16);
	gPlayersQuads[id][BODY]->SetHotSpot(16,8);
	gPlayersQuads[id][HEAD] = new JQuad(playersTexture,offsetX+16,16,16,16);
	gPlayersQuads[id][HEAD]->SetHotSpot(8,7);
	gPlayersQuads[id][RIGHTARM] = new JQuad(playersTexture,offsetX+8,16,8,16);
	gPlayersQuads[id][RIGHTARM]->SetHotSpot(4.0f,4.0f);
	gPlayersQuads[id][RIGHTHAND] = new JQuad(playersTexture,offsetX,16,8,16);
	gPlayersQuads[id][RIGHTHAND]->SetHotSpot(4.0f,3.0f);
	gPlayersQuads[id][LEFTARM] = new JQuad(playersTexture,offsetX+8,16,8,16);
	gPlayersQuads[id][LEFTARM]->SetHotSpot(4.0f,4.0f);
	gPlayersQuads[id][LEFTARM]->SetHFlip(true);
	gPlayersQuads[id][LEFTHAND] = new JQuad(playersTexture,offsetX,16,8,16);
	gPlayersQuads[id][LEFTHAND]->SetHotSpot(4.0f,3.0f);
	gPlayersQuads[id][LEFTHAND]->SetHFlip(true);
	gPlayersQuads[id][LEGS] = new JQuad(playersTexture,offsetX,32,32,32);
	gPlayersQuads[id][LEGS]->SetHotSpot(16,16);
	gPlayersDeadQuads[id] = new JQuad(playersDeadTexture,offsetX,0,29,47);
	gPlayersDeadQuads[id]->SetHotSpot(14,23);

	gPlayerSkinLoaded[id] = true;
	gPlayerSkinTeams[id] = team;
	strncpy(gPlayerSkinNames[id],name,PLAYER_SKIN_NAME_LENGTH-1);
	gPlayerSkinNames[id][PLAYER_SKIN_NAME_LENGTH-1] = '\0';
	gTeamSkinIds[team][gTeamSkinCounts[team]++] = id;
	return true;
}

static void AddLegacyPlayerSkins(int team, JTexture* playersTextures[], JTexture* playersDeadTextures[])
{
	static char* ctNames[] = {"SEAL TEAM 6","GSG-9","SAS","GIGN"};
	static char* tNames[] = {"PHOENIX CONNEXION","ELITE CREW","ARCTIC AVENGERS","GUERILLA WARFARE"};
	for (int i=0; i<4; i++) {
		int id = team == CT ? i : i+4;
		if (gPlayerSkinLoaded[id]) {
			for (id=0; id<MAX_PLAYER_SKINS && gPlayerSkinLoaded[id]; id++) {}
			if (id >= MAX_PLAYER_SKINS) return;
		}
		int atlasId = team == CT ? i+4 : i;
		AddPlayerSkin(id,team,atlasId,team == CT ? ctNames[i] : tNames[i],playersTextures,playersDeadTextures);
	}
}

static void LoadPlayerSkins(JTexture* playersTextures[], JTexture* playersDeadTextures[])
{
	strcpy(gTeamNames[T],"Zombies");
	strcpy(gTeamNames[CT],"UN Forces");
	gTeamSkinCounts[T] = 0;
	gTeamSkinCounts[CT] = 0;
	for (int id=0; id<MAX_PLAYER_SKINS; id++) {
		gPlayerSkinLoaded[id] = false;
		gPlayerSkinTeams[id] = NONE;
		gPlayerSkinNames[id][0] = '\0';
		gPlayersDeadQuads[id] = NULL;
		for (int part=0; part<NUM_QUADS; part++) gPlayersQuads[id][part] = NULL;
	}

	FILE* file = fopen("data/players.txt","r");
	char line[256];
	while (file != NULL && fgets(line,sizeof(line),file) != NULL) {
		char* text = SkipPlayerDataWhitespace(line);
		if (*text == '\0' || *text == '\r' || *text == '\n' || *text == '#') continue;
		int team = 0;
		int consumed = 0;
		if (sscanf(text,"team %d %n",&team,&consumed) == 1 && team >= T && team <= CT) {
			char* name = SkipPlayerDataWhitespace(text+consumed);
			TrimPlayerDataText(name);
			if (name[0] != '\0') {
				strncpy(gTeamNames[team],name,TEAM_NAME_LENGTH-1);
				gTeamNames[team][TEAM_NAME_LENGTH-1] = '\0';
			}
			continue;
		}
		int id = 0;
		int atlasId = 0;
		consumed = 0;
		if (sscanf(text,"player %d %d %d %n",&id,&team,&atlasId,&consumed) == 3) {
			char* name = SkipPlayerDataWhitespace(text+consumed);
			TrimPlayerDataText(name);
			AddPlayerSkin(id,team,atlasId,name,playersTextures,playersDeadTextures);
		}
	}
	if (file != NULL) fclose(file);
	if (gTeamSkinCounts[CT] == 0) AddLegacyPlayerSkins(CT,playersTextures,playersDeadTextures);
	if (gTeamSkinCounts[T] == 0) AddLegacyPlayerSkins(T,playersTextures,playersDeadTextures);
}

GameStateLoading::GameStateLoading(GameApp* parent): GameState(parent) {}

GameStateLoading::~GameStateLoading() {
}


void GameStateLoading::Start()
{
	mStage = 0;
	LoadClientDataConfigs();
	LoadHudDisplayOptions();
	mRenderer->ResetPrivateVRAM();

	mRenderer->EnableVSync(true);

	JTexture* bgTexture = LoadConfiguredTexture(mRenderer,"background","gfx/bg.png",false);					// load bg
	gBgQuad = new JQuad(bgTexture, 0.0f, 0.0f, 480.0f, 272.0f);
	JTexture* logoTexture = LoadConfiguredTexture(mRenderer,"logo","gfx/logo.png",false);				// load logo
	gLogoQuad = new JQuad(logoTexture, 0.0f, 0.0f, 334.0f, 128.0f);
}


void GameStateLoading::End()
{
	mRenderer->EnableVSync(false);

}


void GameStateLoading::Update(float dt)
{
	/*if (mStage == 1)
	{
		mParent->LoadGameStates();
	}
	else if (mStage == 2)
	{
		mParent->SetNextState(GAME_STATE_MENU);
	}

	mStage++;*/

	mStage = Load(mStage);
	if (mStage == 8) {
		mParent->SetNextState(GAME_STATE_MENU);
	}
}


void GameStateLoading::Render()
{
	mRenderer->ClearScreen(ARGB(255,0,0,0));

	float p = ((float)mStage/8);

	mRenderer->RenderQuad(gBgQuad,0,0);
	mRenderer->RenderQuad(gLogoQuad,0,0);
	mRenderer->FillRect(SCREEN_WIDTH*p,0,SCREEN_WIDTH*(1-p),SCREEN_HEIGHT,ARGB(200,0,0,0));

	/*mRenderer->FillRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,ARGB(255-(int)(p*255),0,0,0));
	mRenderer->FillRect(SCREEN_WIDTH_2-200,SCREEN_HEIGHT_2-4,400,8,ARGB(255,100,100,100));
	mRenderer->FillRect(SCREEN_WIDTH_2-200,SCREEN_HEIGHT_2-4,400*p,8,ARGB(255,255,255,255));*/


}

int GameStateLoading::Load(int stage) {
	switch (stage) {
		case 0: {
			JTexture* playersTextures[4];
			JTexture* playersDeadTextures[4];
			playersTextures[0] = LoadConfiguredTexture(mRenderer,"players","gfx/players.png",true);
			playersDeadTextures[0] = LoadConfiguredTexture(mRenderer,"players_dead","gfx/playersdead.png",true);
			for (int page=1; page<4; page++) {
				char playerKey[32];
				char deadKey[32];
				sprintf(playerKey,"players_page_%d",page+1);
				sprintf(deadKey,"players_dead_page_%d",page+1);
				playersTextures[page] = LoadOptionalConfiguredTexture(mRenderer,playerKey,true);
				playersDeadTextures[page] = LoadOptionalConfiguredTexture(mRenderer,deadKey,true);
			}
			LoadPlayerSkins(playersTextures,playersDeadTextures);

			JTexture* radarTexture = LoadConfiguredTexture(mRenderer,"radar","gfx/radar.png",true);
			gRadarQuad = new JQuad(radarTexture,0,0,64,64);
			//gRadarQuad->SetColor(ARGB(175,255,255,255));

			JTexture* buyzoneTexture = LoadConfiguredTexture(mRenderer,"buyzone","gfx/buyzone.png",true);
			gBuyZoneQuad = new JQuad(buyzoneTexture,0,0,16,16);
			gBuyZoneQuad->SetColor(ARGB(175,255,255,255));

			JTexture* decalTexture = LoadConfiguredTexture(mRenderer,"decals","gfx/decals.png",true);
			for (int i=0; i<5; i++) {
				gDecalQuads[i] = new JQuad(decalTexture,i*32.0f,0.0f,32.0f,32.0f);
				gDecalQuads[i]->SetHotSpot(16.0f,16.0f);
				if (i == 0) { //explosion
					gDecalQuads[i]->SetColor(ARGB(255,0,0,0));
				}
				else { //blood
					gDecalQuads[i]->SetColor(ARGB(255,200,0,0));
				}
			}

			gIconTexture = mRenderer->CreateTexture(10,10);
			gIconQuad = new JQuad(gIconTexture,0,0,10,10);
			break;
		}
		case 1: {
			JTexture* gunsTextures[2];
			JTexture* gunsGroundTextures[2];
			gunsTextures[0] = LoadConfiguredTexture(mRenderer,"guns_page_1","gfx/guns.png",true);
			gunsTextures[1] = LoadConfiguredTexture(mRenderer,"guns_page_2","gfx/guns2.png",true);
			gunsGroundTextures[0] = LoadConfiguredTexture(mRenderer,"guns_ground_page_1","gfx/gunsground.png",true);
			gunsGroundTextures[1] = LoadConfiguredTexture(mRenderer,"guns_ground_page_2","gfx/gunsground2.png",true);

			gGunHandQuads = new JQuad*[MAX_GUNS];
			gGunGroundQuads = new JQuad*[MAX_GUNS];
			for (int i=0; i<MAX_GUNS; i++) {
				gGunHandQuads[i] = NULL;
				gGunGroundQuads[i] = NULL;
			}

			for (int page=0; page<2; page++) {
				int localId = 0;
				for (int i=0; i<(gunsTextures[page]->mTexHeight)/32 && localId<GUNS_PER_ATLAS; i++) {
					for (int j=0; j<(gunsTextures[page]->mTexWidth)/32 && localId<GUNS_PER_ATLAS; j++) {
						int id = page*GUNS_PER_ATLAS+localId;
						gGunHandQuads[id] = new JQuad(gunsTextures[page],j*32.0f,i*32.0f,32.0f,32.0f);
						gGunHandQuads[id]->SetHotSpot(16.0f,8.0f);
						gGunGroundQuads[id] = new JQuad(gunsGroundTextures[page],j*32.0f,i*32.0f,32.0f,32.0f);
						gGunGroundQuads[id]->SetHotSpot(16.0f,16.0f);
						localId++;
					}
				}
			}

			JTexture* muzzleFlashTexture = LoadConfiguredTexture(mRenderer,"muzzle_flash","gfx/muzzleflash.png",true);
			gMuzzleFlashQuads.clear();
			gMuzzleFlashQuads[0].resize(MUZZLE_FLASH_FRAMES,NULL);
			for (int frame=0; frame<MUZZLE_FLASH_FRAMES; frame++) {
				JQuad* quad = new JQuad(muzzleFlashTexture,frame*32,0,32,32);
				quad->SetHotSpot(16.0f,-16.0f);
				gMuzzleFlashQuads[0][frame] = quad;
			}
			FILE* resourceFile = fopen("data/resources.txt","r");
			std::vector<int> muzzleFlashTypes;
			char resourceLine[1024];
			while (resourceFile != NULL && fgets(resourceLine,sizeof(resourceLine),resourceFile) != NULL) {
				int type = 0;
				if (sscanf(resourceLine,"muzzle_flash_%d",&type) == 1 && type > 0) muzzleFlashTypes.push_back(type);
			}
			if (resourceFile != NULL) fclose(resourceFile);
			std::sort(muzzleFlashTypes.begin(),muzzleFlashTypes.end());
			muzzleFlashTypes.erase(std::unique(muzzleFlashTypes.begin(),muzzleFlashTypes.end()),muzzleFlashTypes.end());
			for (unsigned int typeIndex=0; typeIndex<muzzleFlashTypes.size(); typeIndex++) {
				int type = muzzleFlashTypes[typeIndex];
				char key[32];
				sprintf(key,"muzzle_flash_%d",type);
				char* configured = GetConfig("data/resources.txt",key);
				JTexture* texture = configured == NULL ? NULL : mRenderer->LoadTexture(configured,true);
				delete[] configured;
				gMuzzleFlashQuads[type].resize(MUZZLE_FLASH_FRAMES,NULL);
				for (int frame=0; frame<MUZZLE_FLASH_FRAMES; frame++) {
					if (texture != NULL) {
						gMuzzleFlashQuads[type][frame] = new JQuad(texture,frame*32,0,32,32);
						gMuzzleFlashQuads[type][frame]->SetHotSpot(16.0f,-16.0f);
					}
				}
			}

			JTexture* healthTexture = LoadConfiguredTexture(mRenderer,"health_hud","gfx/health.png",true);
			gHealthBorderQuad = new JQuad(healthTexture,0,0,48,48);
			gHealthFillQuad = new JQuad(healthTexture,48,0,48,48);
			gHealthPickupQuad = new JQuad(healthTexture,0,0,128,64);
			gHealthPickupQuad->SetHotSpot(64,32);

			JTexture* armorTexture = LoadConfiguredTexture(mRenderer,"armor_hud","gfx/armor.png",true);
			gArmorBorderQuad = new JQuad(armorTexture,0,0,48,48);
			gArmorFillQuad = new JQuad(armorTexture,48,0,48,48);
			gArmorPickupQuad = new JQuad(armorTexture,0,0,128,64);
			gArmorPickupQuad->SetHotSpot(64,32);
			JTexture* armorGroundTexture = LoadConfiguredTexture(mRenderer,"armor_ground","gfx/armorground.png",true);
			gArmorGroundQuad = new JQuad(armorGroundTexture,0,0,32,32);
			gArmorGroundQuad->SetHotSpot(16,16);
			JTexture* healthGroundTexture = LoadConfiguredTexture(mRenderer,"health_ground","gfx/healthground.png",true);
			gHealthGroundQuad = new JQuad(healthGroundTexture,0,0,32,32);
			gHealthGroundQuad->SetHotSpot(16,16);
			JTexture* ammoGroundTexture = LoadConfiguredTexture(mRenderer,"ammo_ground","gfx/ammoground.png",true);
			gAmmoGroundQuad = new JQuad(ammoGroundTexture,0,0,32,32);
			gAmmoGroundQuad->SetHotSpot(16,16);

			JTexture* ammobarTexture = LoadConfiguredTexture(mRenderer,"ammo_hud","gfx/ammo.png",true);
			gAmmoBarQuad = new JQuad(ammobarTexture,0,0,128,64);
			gAmmoBarQuad->SetHotSpot(128,64);
			gAmmoPickupQuad = new JQuad(ammobarTexture,0,0,128,64);
			gAmmoPickupQuad->SetHotSpot(64,32);

			JTexture* damageIndicatorTexture = LoadConfiguredTexture(mRenderer,"damage_indicator","gfx/damageindicator.png",true);
			gDamageIndicator = new JQuad(damageIndicatorTexture,0,0,128,32);
			gDamageIndicator->SetHotSpot(64,125);
			gDamageIndicator->SetColor(ARGB(128,255,255,255));

			JTexture* scoreIconsTexture = LoadConfiguredTexture(mRenderer,"score_icons","gfx/scoreicons.png",true);
			for (int i=0; i<4; i++) {
				gScoreIconQuads[i] = new JQuad(scoreIconsTexture,16*i+3,2,10,13);
				gScoreIconQuads[i]->SetHotSpot(5,6);
			}

			JTexture* ctfTexture = LoadConfiguredTexture(mRenderer,"ctf","gfx/ctf.png",true);
			gFlagQuad = new JQuad(ctfTexture,0,0,32,32);
			gFlagQuad->SetHotSpot(16,16);
			gFlagHomeQuad = new JQuad(ctfTexture,32,0,32,32);
			gFlagHomeQuad->SetHotSpot(16,16);
			gFlagArrowQuad = new JQuad(ctfTexture,65,0,30,16);
			gFlagArrowQuad->SetHotSpot(15,22);
			gFlagRadarQuad = new JQuad(ctfTexture,96,0,8,8);
			gFlagRadarQuad->SetHotSpot(4,4);
			gHomeRadarQuad = new JQuad(ctfTexture,96+8,0,8,8);
			gHomeRadarQuad->SetHotSpot(4,4);
			break;
		}
		case 2: {
			gDryFireRifleSound = LoadConfiguredSample(mSoundSystem,"dryfire_rifle","sfx/dryfire_rifle.wav");
			gDryFirePistolSound = LoadConfiguredSample(mSoundSystem,"dryfire_pistol","sfx/dryfire_pistol.wav");
			gDeploySound = LoadConfiguredSample(mSoundSystem,"deploy","sfx/deploy.wav");
			gPickUpSound = LoadConfiguredSample(mSoundSystem,"pickup","sfx/pickup.wav");
			gAmmoSound = LoadConfiguredSample(mSoundSystem,"ammo","sfx/ammo.wav");
			gPinPullSound = LoadConfiguredSample(mSoundSystem,"pin_pull","sfx/pinpull.wav");
			gWalkSounds[0] = LoadConfiguredSample(mSoundSystem,"walk_1","sfx/walk1.wav");
			gWalkSounds[1] = LoadConfiguredSample(mSoundSystem,"walk_2","sfx/walk2.wav");
			gRicochetSounds[0] = LoadConfiguredSample(mSoundSystem,"ricochet_1","sfx/ricochet1.wav");
			gRicochetSounds[1] = LoadConfiguredSample(mSoundSystem,"ricochet_2","sfx/ricochet2.wav");
			gRicochetSounds[2] = LoadConfiguredSample(mSoundSystem,"ricochet_3","sfx/ricochet3.wav");
			gRicochetSounds[3] = LoadConfiguredSample(mSoundSystem,"ricochet_4","sfx/ricochet4.wav");
			gHitSounds[0] = LoadConfiguredSample(mSoundSystem,"hit_1","sfx/hit1.wav");
			gHitSounds[1] = LoadConfiguredSample(mSoundSystem,"hit_2","sfx/hit2.wav");
			gHitSounds[2] = LoadConfiguredSample(mSoundSystem,"hit_3","sfx/hit3.wav");
			break;
		}	
		case 3: {
			gKnifeHitSound = LoadConfiguredSample(mSoundSystem,"knife_hit","sfx/knifehit.wav");
			gZombieClawsHitSound = LoadConfiguredSampleWithLegacyKey(mSoundSystem,"zombie_claws_hit","knife_hit","sfx/knifehit.wav");
			gDieSounds[CT][0] = LoadConfiguredSampleWithLegacyKey(mSoundSystem,"ct_die_1","die_1","sfx/die1.wav");
			gDieSounds[CT][1] = LoadConfiguredSampleWithLegacyKey(mSoundSystem,"ct_die_2","die_2","sfx/die2.wav");
			gDieSounds[CT][2] = LoadConfiguredSampleWithLegacyKey(mSoundSystem,"ct_die_3","die_3","sfx/die3.wav");
			gDieSounds[T][0] = LoadConfiguredSampleWithLegacyKey(mSoundSystem,"t_die_1","die_1","sfx/die1.wav");
			gDieSounds[T][1] = LoadConfiguredSampleWithLegacyKey(mSoundSystem,"t_die_2","die_2","sfx/die2.wav");
			gDieSounds[T][2] = LoadConfiguredSampleWithLegacyKey(mSoundSystem,"t_die_3","die_3","sfx/die3.wav");
			gRoundEndSounds[T] = LoadConfiguredSample(mSoundSystem,"t_win","sfx/twin.wav");
			gRoundEndSounds[CT] = LoadConfiguredSample(mSoundSystem,"ct_win","sfx/ctwin.wav");
			gRoundEndSounds[TIE] = LoadConfiguredSample(mSoundSystem,"round_draw","sfx/rounddraw.wav");
			gHEGrenadeSounds[0] = LoadConfiguredSample(mSoundSystem,"he_grenade_1","sfx/hegrenade1.wav");
			gHEGrenadeSounds[1] = LoadConfiguredSample(mSoundSystem,"he_grenade_2","sfx/hegrenade2.wav");
			gHEGrenadeSounds[2] = LoadConfiguredSample(mSoundSystem,"he_grenade_3","sfx/hegrenade3.wav");
			gFlashbangSound = LoadConfiguredSample(mSoundSystem,"flashbang","sfx/flashbang.wav");
			gSmokeGrenadeSound = LoadConfiguredSample(mSoundSystem,"smoke_grenade","sfx/smokegrenade.wav");
			gFireInTheHoleSound = LoadConfiguredSample(mSoundSystem,"fire_in_the_hole","sfx/fireinthehole.wav");
			gGrenadeBounceSound = LoadConfiguredSample(mSoundSystem,"grenade_bounce","sfx/grenadebounce.wav");
			gHitIndicatorSound = LoadConfiguredSample(mSoundSystem,"hit_indicator","sfx/hitindicator.wav");
			break;
		}

		case 4: {
			gKeyFrameAnims[ANIM_PRIMARY] = Animation::LoadKeyFrames("PRIMARY");
			gKeyFrameAnims[ANIM_PRIMARY_FIRE] = Animation::LoadKeyFrames("PRIMARY_FIRE");
			gKeyFrameAnims[ANIM_SECONDARY] = Animation::LoadKeyFrames("SECONDARY");
			gKeyFrameAnims[ANIM_SECONDARY_FIRE] = Animation::LoadKeyFrames("SECONDARY_FIRE");
			gKeyFrameAnims[ANIM_KNIFE] = Animation::LoadKeyFrames("KNIFE");
			gKeyFrameAnims[ANIM_KNIFE_SLASH] = Animation::LoadKeyFrames("KNIFE_SLASH");
			gKeyFrameAnims[ANIM_GRENADE] = Animation::LoadKeyFrames("GRENADE");
			gKeyFrameAnims[ANIM_GRENADE_PULLBACK] = Animation::LoadKeyFrames("GRENADE_PULLBACK");
			gKeyFrameAnims[ANIM_PRIMARY_RELOAD] = Animation::LoadKeyFrames("PRIMARY_RELOAD");
			gKeyFrameAnims[ANIM_SECONDARY_RELOAD] = Animation::LoadKeyFrames("SECONDARY_RELOAD");
			break;
		}		
		case 5: {
			int mNumGuns = 0;
			FILE *file;
			file = fopen("data/guns.txt", "r"); 
			char line[1024]; 
			fgets(line,1024,file);  // Get one line from your file into a buffer 
			sscanf(line,"%d",&mNumGuns);  // read mCols and mRows of your map from the file 
			if (mNumGuns < 0 || mNumGuns > MAX_GUNS) {
				fclose(file);
				break;
			}
			//mGuns = new Gun*[mNumGuns];
			char* s = line; 

			for (int i=0;i<mNumGuns;) { 
				if (!fgets(line,1024,file)) break; // read error, you should handle this properly 
				s = line; // This was what the problem was! 
				Gun gun = {};
				int fields = sscanf(s,"%d %d %d %f %d %d %d %f %f %f %d %d %d %d %d %d %d %d %14s",&gun.mId,&gun.mDamage,&gun.mDelay,&gun.mSpread,&gun.mClip,&gun.mNumClips,&gun.mReloadDelay,&gun.mSpeed,&gun.mBulletSpeed,&gun.mViewAngle,&gun.mCost,&gun.mType,&gun.mFireMode,&gun.mPellets,&gun.mScope,&gun.mBuyCategory,&gun.mBuyTeams,&gun.mMuzzleFlashType,gun.mName);
				if (fields != 19) {
					gun.mMuzzleFlashType = 0;
					fields = sscanf(s,"%d %d %d %f %d %d %d %f %f %f %d %d %d %d %d %d %d %14s",&gun.mId,&gun.mDamage,&gun.mDelay,&gun.mSpread,&gun.mClip,&gun.mNumClips,&gun.mReloadDelay,&gun.mSpeed,&gun.mBulletSpeed,&gun.mViewAngle,&gun.mCost,&gun.mType,&gun.mFireMode,&gun.mPellets,&gun.mScope,&gun.mBuyCategory,&gun.mBuyTeams,gun.mName);
					if (fields != 18) continue;
				}
				if (gun.mId < 0 || gun.mId >= MAX_GUNS) continue;
				if (gun.mMuzzleFlashType < 0) gun.mMuzzleFlashType = 0;
				if (gun.mPellets < 1) gun.mPellets = 1;
				if (gun.mPellets > MAX_PELLETS) gun.mPellets = MAX_PELLETS;
				if (gun.mScope < SCOPE_NONE || gun.mScope > SCOPE_HIGH) gun.mScope = SCOPE_NONE;
				if (gun.mBuyCategory < BUY_CATEGORY_NONE || gun.mBuyCategory > BUY_CATEGORY_EQUIPMENT) gun.mBuyCategory = BUY_CATEGORY_NONE;
				if (gun.mBuyTeams < 0 || gun.mBuyTeams > (BUY_TEAM_T | BUY_TEAM_CT)) gun.mBuyTeams = 0;
				gun.mHandQuad = gGunHandQuads[gun.mId];
				gun.mGroundQuad = gGunGroundQuads[gun.mId];
				gGuns[i] = gun;

				//strcpy(gGuns[i].mName,name);
				char buffer[128];

				if (gGuns[i].mId == 28) {
					gGuns[i].mFireSound = mSoundSystem->LoadSample("sfx/m4a1.wav"); // AS-VAL (ID 28) -> M4A1 (ID 17)
				}
				else if (gGuns[i].mId == 29) {
					gGuns[i].mFireSound = mSoundSystem->LoadSample("sfx/mp5.wav"); // KRISS-VECTOR (ID 29) -> MP5 (ID 11)
				}
				else if (gGuns[i].mId == 30) {
					gGuns[i].mFireSound = mSoundSystem->LoadSample("sfx/m249.wav"); // BAR (ID 30) -> M249 (ID 24)
				}
				else if (gGuns[i].mId == 31) {
					gGuns[i].mFireSound = mSoundSystem->LoadSample("sfx/glock.wav"); // GLOCK18AUTO (ID 31) -> GLOCK (ID 1)
				}
				/*else if (gGuns[i].mId == 32) {
					gGuns[i].mFireSound = mSoundSystem->LoadSample("sfx/ak47.wav"); // ID 32 reuses the AK-47 fire sound
				}*/
				//CZ fire sound
				else if (gGuns[i].mId == 33) {
					gGuns[i].mFireSound = mSoundSystem->LoadSample("sfx/mp5.wav"); // ID 32 reuses the AK-47 fire sound
				}
				//tec-9 fire sound
				else if (gGuns[i].mId == 32) {
					gGuns[i].mFireSound = mSoundSystem->LoadSample("sfx/glock.wav"); // ID 32 reuses the AK-47 fire sound
				}
				else if (gGuns[i].mId == 25 || gGuns[i].mId == 26 || gGuns[i].mId == 27) {
					gGuns[i].mFireSound = gPinPullSound;
				}
				else {
					sprintf(buffer,"sfx/%s.wav",gGuns[i].mName);
					gGuns[i].mFireSound = mSoundSystem->LoadSample(buffer);
				}

				if (gGuns[i].mId == 28) {
					gGuns[i].mReloadSound = mSoundSystem->LoadSample("sfx/m4a1reload.wav"); // AS-VAL (ID 28) -> M4A1 (ID 17)
				}
				else if (gGuns[i].mId == 29) {
					gGuns[i].mReloadSound = mSoundSystem->LoadSample("sfx/mp5reload.wav"); // KRISS-VECTOR (ID 29) -> MP5 (ID 11)
				}
				else if (gGuns[i].mId == 30) {
					gGuns[i].mReloadSound = mSoundSystem->LoadSample("sfx/m249reload.wav"); // BAR (ID 30) -> M249 (ID 24)
				}
				else if (gGuns[i].mId == 31) {
					gGuns[i].mReloadSound = mSoundSystem->LoadSample("sfx/glockreload.wav"); // GLOCK18AUTO (ID 31) -> GLOCK (ID 1)
				}
				//attach sounds to the new guns you want using the else if here - add gun step 2
				/*else if (gGuns[i].mId == 32) {
					gGuns[i].mReloadSound = mSoundSystem->LoadSample("sfx/ak47reload.wav"); // ID 32 reuses the AK-47 reload sound
				}*/
				//CZ reload sound
				else if (gGuns[i].mId == 33) {
					gGuns[i].mReloadSound = mSoundSystem->LoadSample("sfx/glockreload.wav"); // ID 32 reuses the AK-47 reload sound
				}
				//tec-9 reload sound
				else if (gGuns[i].mId == 32) {
					gGuns[i].mReloadSound = mSoundSystem->LoadSample("sfx/ak47reload.wav"); // ID 32 reuses the AK-47 reload sound
				}
				else if (gGuns[i].mId != 0 && gGuns[i].mId != 25 && gGuns[i].mId != 26 && gGuns[i].mId != 27) {
					sprintf(buffer,"sfx/%sreload.wav",gGuns[i].mName);
					gGuns[i].mReloadSound = mSoundSystem->LoadSample(buffer);
				}

				//mGuns[i] = new Gun(gGunHandQuads[a],gGunGroundQuads[a],a,c,b,d,e,f,g,h,l,m,name);
				if (gGuns[i].mType == PRIMARY) {
					gGuns[i].mDryFireSound = gDryFireRifleSound;
				}
				else if (gGuns[i].mType == SECONDARY) {
					gGuns[i].mDryFireSound = gDryFirePistolSound;
				}

				int clip = gGuns[i].mClip;
				int space = 120-clip;
				int w = 0; 
				if (clip != 0) {
					w = (int)floorf((float)space/clip);
				}
				if (w < 0) w = 0;
				gGuns[i].mAmmoBarWidth = w;

				//mGuns[i] = gGuns[i];
				i++;
			}

			for (int i=0; i<mNumGuns; i++) {
				JSample* fallbackFireSound = gGuns[0].mFireSound;
				JSample* fallbackReloadSound = gGuns[0].mReloadSound;
				if (gGuns[i].mType == PRIMARY) {
					fallbackFireSound = gGuns[18].mFireSound;
					fallbackReloadSound = gGuns[18].mReloadSound;
				}
				else if (gGuns[i].mType == SECONDARY) {
					fallbackFireSound = gGuns[1].mFireSound;
					fallbackReloadSound = gGuns[1].mReloadSound;
				}
				else if (gGuns[i].mType == GRENADE) {
					fallbackFireSound = gPinPullSound;
					fallbackReloadSound = NULL;
				}

				if (gGuns[i].mFireSound == NULL || gGuns[i].mFireSound->mSample == NULL) {
					if (gGuns[i].mFireSound != NULL && gGuns[i].mFireSound != fallbackFireSound) {
						delete gGuns[i].mFireSound;
					}
					gGuns[i].mFireSound = fallbackFireSound;
				}
				if (gGuns[i].mReloadSound == NULL || gGuns[i].mReloadSound->mSample == NULL) {
					if (gGuns[i].mReloadSound != NULL && gGuns[i].mReloadSound != fallbackReloadSound) {
						delete gGuns[i].mReloadSound;
					}
					gGuns[i].mReloadSound = fallbackReloadSound;
				}
			}
			fclose(file);
			Bullet::LoadTracerConfig("data/tracers.txt");
			LoadLaserConfigs("data/lasers.txt");

			break;
		}
		case 6: {
			//gTTFont = new JTTFont();
			//gTTFont->Load("data/tahomabd.ttf",20);
			//gTTFont->PreCacheASCII();

			gFont = new JLBFont("data/f3", 16, true);
			gFont->SetBase(0);
			//gFontBackdrop = new JLBFont("data/f3backdrop", 16, true);
			//gFontBackdrop->SetBase(0);
			//gHudFont = new JLBFont("data/hud", 17, true);
			//gHudFont->SetBase(0);
			//gHudFont->SetBlendMode(BLEND_COLORADD);

			JTexture* particlesTexture = LoadConfiguredTexture(mRenderer,"particles","gfx/particles.png",true);

			JQuad* quad = new JQuad(particlesTexture,32,0,32,32);
			quad->SetHotSpot(16.0f, 16.0f);
			gParticleEngine = new ParticleEngine(gEffectsConfig.particlePoolSize);
			gParticleEngine->SetQuad(quad);

			quad = new JQuad(particlesTexture,32,0,32,32);
			quad->SetHotSpot(16.0f, 16.0f);
			gParticleSystems[PARTICLE_EXPLOSION] = new hgeParticleSystem("gfx/explosion.psi",quad);
			quad = new JQuad(particlesTexture,64,64,32,32);
			quad->SetHotSpot(16.0f, 16.0f);
			gParticleSystems[PARTICLE_FLASH] = new hgeParticleSystem("gfx/flash.psi",quad);
			quad = new JQuad(particlesTexture,0,96,32,32);
			quad->SetHotSpot(16.0f, 16.0f);
			gParticleSystems[PARTICLE_SMOKE] = new hgeParticleSystem("gfx/smoke.psi",quad);

			
			gSfxManager = new SfxManager();
			//mParticles = new JParticleSystem(500);
			//mParticles->SetQuad(mParticlesQuad);
			gDanzeff = new Danzeff();

			gSocket = new Socket();
			gHttpManager = new HttpManager();

			gLastKey = 0;
			gKeyRepeatDelay = 0;
			break;
		}

		case 7: {
			mParent->LoadGameStates();
			break;
		}
	}

	return ++stage;

}
