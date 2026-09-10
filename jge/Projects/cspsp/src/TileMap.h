#ifndef _TILEMAP_H_
#define _TILEMAP_H_

#include "JRenderer.h"
#include "Node.h"
#include "GunObject.h"
#include "Collision.h"

#define DECAL_BLOOD 0
#define DECAL_EXPLOSION 1

#define TEAM 0
#define FFA 1
#define CTF 2
#define EXTERMINATION 3
#define INFECTION 4
#define HORDE 5

struct CollisionPoint {
	float x;
	float y;
	bool bullets;
	bool people;
};

struct CollisionLine
{
	Line line;
	bool bullets;
	bool people;

	CollisionLine(Line _line): line(_line) {}
	~CollisionLine() {}
};

struct Decal {
	JQuad* quad;
	float x;
	float y;
	float scale;
	float angle;
};

struct BuyZone {
	float x1;
	float y1;
	float x2;
	float y2;
};

#define PICKUP_HEALTH 0
#define PICKUP_ARMOR 1
#define PICKUP_AMMO 2

struct MapPickup {
	int type;
	int id;
	float x;
	float y;
	int amount;
	float respawnTime;
	float respawnTimer;
	bool active;
};

//------------------------------------------------------------------------------------------------
class TileMap
{
private:
	static JRenderer* mRenderer;
	std::vector<JQuad*> mTiles;
	std::vector<int> gMap;
	JTexture *mTexture;
	JTexture *mOverviewTexture;
	bool loaded;
	int mBackgroundTile;

	std::vector<Decal> mDecals;

protected:

public:
	char mName[20];
	int mCols;
	int mRows;
	int mOverviewWidth;
	int mOverviewHeight;

	std::vector<CollisionLine> mCollisionLines;
	std::vector<CollisionPoint> mCollisionPoints;

	std::vector<Vector2D*> mCTSpawns;
	std::vector<Vector2D*> mTSpawns;
	std::vector<BuyZone> mCTBuyZones;
	std::vector<BuyZone> mTBuyZones;
	std::vector<Node*> mNodes;
	std::vector<MapPickup> mPickupSpawns;
	std::vector<MapPickup> mPickups;
	int mNumPoints;
	int mNumCTs;
	int mNumTs;
	std::vector<GunObject*>* mGunObjects;
	std::vector<GunObject*> mGunObjectsSpawn;
	JQuad* mOverviewQuad;

	Vector2D mFlagSpawn[2];

	TileMap(std::vector<GunObject*>* gunobjects);
	~TileMap();
	bool Load(char *mapFile, Gun guns[], int gameType = TEAM);
	void Unload();
	void Update(float dt);
	void Render(float x, float y);
	void Reset();

	void AddDecal(float x, float y, int type);
	void ClearDecals();
};

#endif
