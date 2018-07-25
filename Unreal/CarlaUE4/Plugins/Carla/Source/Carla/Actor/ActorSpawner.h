// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Actor/ActorDescription.h"
#include "Carla/Actor/ActorSpawnResult.h"

#include "Containers/Array.h"
#include "GameFramework/Actor.h"

#include "ActorSpawner.generated.h"

/// Base class for Carla actor spawners.
UCLASS(Abstract)
class CARLA_API AActorSpawner : public AActor
{
  GENERATED_BODY()

public:

  AActorSpawner(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
  {
    PrimaryActorTick.bCanEverTick = false;
  }

  /// Retrieve the list of actor definitions that this class is able to spawn.
  virtual TArray<FActorDefinition> MakeDefinitions() {
    unimplemented();
    return {};
  }

  /// Spawn an actor based on @a ActorDescription and @a Transform.
  ///
  /// @pre ActorDescription is expected to be derived from one of the
  /// definitions retrieved with MakeDefinitions.
  virtual FActorSpawnResult SpawnActor(
      const FTransform &SpawnAtTransform,
      const FActorDescription &ActorDescription) {
    unimplemented();
    return {};
  }
};
