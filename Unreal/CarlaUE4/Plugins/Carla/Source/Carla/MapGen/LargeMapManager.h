// Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Engine/LevelStreamingDynamic.h"

#include "Math/DVector.h"

#include "LargeMapManager.generated.h"


// TODO: Cache CarlaEpisode

USTRUCT()
struct FGhostActor
{
  GENERATED_BODY()

  FGhostActor() {}

  FGhostActor(
    const FActorView* InActorView,
    const FTransform& InTransform)
    : ActorView(InActorView),
      WorldLocation(FDVector(InTransform.GetTranslation())),
      Rotation(InTransform.GetRotation()) {}

  const FActorView* ActorView;

  FDVector WorldLocation;

  FQuat Rotation;
};

USTRUCT()
struct FCarlaMapTile
{
  GENERATED_BODY()

  UPROPERTY(VisibleAnywhere, Category = "Carla Map Tile")
  FString Name; // Tile_{TileID_X}_{TileID_Y}

  // Absolute location, does not depend on rebasing
  UPROPERTY(VisibleAnywhere, Category = "Carla Map Tile")
  FVector Location{0.0f};
  // TODO: not FVector

  UPROPERTY(VisibleAnywhere, Category = "Carla Map Tile")
  ULevelStreamingDynamic* StreamingLevel = nullptr;

  bool TilesSpawned = false;
};

UCLASS()
class CARLA_API ALargeMapManager : public AActor
{
  GENERATED_BODY()

public:

  using TileID = uint64;

  // Sets default values for this actor's properties
  ALargeMapManager();

  ~ALargeMapManager();

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

  void PreWorldOriginOffset(UWorld* InWorld, FIntVector InSrcOrigin, FIntVector InDstOrigin);
  void PostWorldOriginOffset(UWorld* InWorld, FIntVector InSrcOrigin, FIntVector InDstOrigin);

  void OnLevelAddedToWorld(ULevel* InLevel, UWorld* InWorld);
  void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld);

public:

  void RegisterInitialObjects();

  void OnActorSpawned(const FActorView& ActorView);

  UFUNCTION(Category="Large Map Manager")
  void OnActorDestroyed(AActor* DestroyedActor);

  // Called every frame
  void Tick(float DeltaTime) override;

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  void GenerateMap(FString InAssetsPath);

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  void GenerateLargeMap();

  void AddActorToUnloadedList(const FActorView& ActorView, const FTransform& Transform);

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  FIntVector GetNumTilesInXY() const;

  UFUNCTION(BlueprintCallable, Category = "Large Map Manager")
  bool IsLevelOfTileLoaded(FIntVector InTileID) const;

  bool IsTileLoaded(TileID TileId) const
  {
    return CurrentTilesLoaded.Contains(TileId);
  }

  bool IsTileLoaded(FVector Location) const
  {
    return IsTileLoaded(GetTileID(Location));
  }

  bool IsTileLoaded(FDVector Location) const
  {
    return IsTileLoaded(GetTileID(Location));
  }

  FTransform GlobalToLocalTransform(const FTransform& InTransform) const;
  FVector GlobalToLocalLocation(const FVector& InLocation) const;

  FTransform LocalToGlobalTransform(const FTransform& InTransform) const;
  FVector LocalToGlobalLocation(const FVector& InLocation) const;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  FString LargeMapTilePath = "/Game/Carla/Maps/testmap";
  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  FString LargeMapName = "testmap";

protected:

  FIntVector GetTileVectorID(FVector TileLocation) const;

  FIntVector GetTileVectorID(FDVector TileLocation) const;

  FIntVector GetTileVectorID(TileID TileID) const;

  FVector GetTileLocation(TileID TileID) const;

  FVector GetTileLocation(FIntVector TileVectorID) const;

  FDVector GetTileLocationD(TileID TileID) const;

  FDVector GetTileLocationD(FIntVector TileVectorID) const;

  /// From a given location it retrieves the TileID that covers that area
  TileID GetTileID(FVector TileLocation) const;

  TileID GetTileID(FDVector TileLocation) const;

  TileID GetTileID(FIntVector TileVectorID) const;

  FCarlaMapTile& GetCarlaMapTile(FVector Location);

  FCarlaMapTile& GetCarlaMapTile(ULevel* InLevel);

  FCarlaMapTile* GetCarlaMapTile(FIntVector TileVectorID);

  ULevelStreamingDynamic* AddNewTile(FString TileName, FVector TileLocation);

  FCarlaMapTile& LoadCarlaMapTile(FString TileMapPath, TileID TileId);

  void UpdateTilesState();

  void RemovePendingActorsToRemove();

  // Check if any ghost actor has to be converted into dormant actor
  // because it went out of range (ActorStreamingDistance)
  // Just stores the array of selected actors
  void CheckGhostActors();

  // Converts ghost actors that went out of range to dormant actors
  void ConvertGhostToDormantActors();

  // Check if any dormant actor has to be converted into ghost actor
  // because it enter in range (ActorStreamingDistance)
  // Just stores the array of selected actors
  void CheckDormantActors();

  // Converts ghost actors that went out of range to dormant actors
  void ConvertDormantToGhostActors();

  void CheckIfRebaseIsNeeded();

  void GetTilesToConsider(
    const AActor* ActorToConsider,
    TSet<TileID>& OutTilesToConsider);

  void GetTilesThatNeedToChangeState(
    const TSet<TileID>& InTilesToConsider,
    TSet<TileID>& OutTilesToBeVisible,
    TSet<TileID>& OutTilesToHidde);

  void UpdateTileState(
    const TSet<TileID>& InTilesToUpdate,
    bool InShouldBlockOnLoad,
    bool InShouldBeLoaded,
    bool InShouldBeVisible);

  void UpdateCurrentTilesLoaded(
    const TSet<TileID>& InTilesToBeVisible,
    const TSet<TileID>& InTilesToHidde);

  UPROPERTY(VisibleAnywhere, Category = "Large Map Manager")
  TMap<uint64, FCarlaMapTile> MapTiles;

  // All actors to be consider for tile loading (all hero vehicles)
  // The first actor in the array is the one selected for rebase
  // TODO: support rebase in more than one hero vehicle
  UPROPERTY(VisibleAnywhere, Category = "Large Map Manager")
  TArray<AActor*> ActorsToConsider;
  //UPROPERTY(VisibleAnywhere, Category = "Large Map Manager")
  TArray<FActorView::IdType> GhostActors;
  TArray<FActorView::IdType> DormantActors;

  // Temporal sets to remove actors. Just to avoid removing them in the update loop
  TSet<AActor*> ActorsToRemove;
  TSet<FActorView::IdType> GhostsToRemove;
  TSet<FActorView::IdType> DormantsToRemove;

  // Helpers to move Actors from one array to another.
  TSet<FActorView::IdType> GhostToDormantActors;
  TSet<FActorView::IdType> DormantToGhostActors;

  TSet<TileID> CurrentTilesLoaded;

  // Current Origin after rebase
  UPROPERTY(VisibleAnywhere, Category = "Large Map Manager")
  FIntVector CurrentOriginInt{ 0 };

  FDVector CurrentOriginD;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  FVector Tile0Offset = FVector(0,0,0);

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float TickInterval = 0.0f;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float LayerStreamingDistance = 3.0f * 1000.0f * 100.0f;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float ActorStreamingDistance = 2.0f * 1000.0f * 100.0f;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float RebaseOriginDistance = 2.0f * 1000.0f * 100.0f;

  float LayerStreamingDistanceSquared = LayerStreamingDistance * LayerStreamingDistance;
  float ActorStreamingDistanceSquared = ActorStreamingDistance * ActorStreamingDistance;
  float RebaseOriginDistanceSquared = RebaseOriginDistance * RebaseOriginDistance;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float TileSide = 2.0f * 1000.0f * 100.0f; // 2km

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  bool ShouldTilesBlockOnLoad = false;

  UFUNCTION(BlueprintCallable, CallInEditor, Category = "Large Map Manager")
    void GenerateMap_Editor()
  {
    if (!LargeMapTilePath.IsEmpty()) GenerateMap(LargeMapTilePath);
  }

  FString GenerateTileName(TileID TileID);

  FString TileIDToString(TileID TileID);

  void DumpTilesTable() const;

  void PrintMapInfo();

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  FString AssetsPath = "";

  FString BaseTileMapPath = "/Game/Carla/Maps/LargeMap/EmptyTileBase";

  FColor PositonMsgColor = FColor::Purple;

  const int32 TilesDistMsgIndex = 100;
  const int32 MaxTilesDistMsgIndex = TilesDistMsgIndex + 10;

  const int32 ClientLocMsgIndex = 200;
  const int32 MaxClientLocMsgIndex = ClientLocMsgIndex + 10;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  float MsgTime = 1.0f;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  bool bPrintMapInfo = true;

  UPROPERTY(EditAnywhere, Category = "Large Map Manager")
  bool bPrintErrors = false;

};
