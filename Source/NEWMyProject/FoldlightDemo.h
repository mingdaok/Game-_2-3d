#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/HUD.h"
#include "GameFramework/SaveGame.h"
#include "FoldlightDemo.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UCameraComponent;
class USpringArmComponent;
class UPointLightComponent;
class UNiagaraComponent;
class AFDWorld;

UENUM()
enum class EFDKind : uint8 { Platform, Pillar, Vine, Receiver, Exit, Decoration };

UCLASS()
class NEWMYPROJECT_API UFDSave : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY() int32 Room = 0;
    UPROPERTY() bool Finished = false;
    UPROPERTY() int32 Version = 1;
};

UCLASS()
class NEWMYPROJECT_API AFDObject : public AActor
{
    GENERATED_BODY()
public:
    AFDObject();
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UBoxComponent> Solid;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UBoxComponent> Projection;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> VisualRoot;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Pieces;
    UPROPERTY() TArray<TObjectPtr<UMaterialInstanceDynamic>> OriginalMaterials;
    UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> Surface;
    UPROPERTY() TObjectPtr<AFDWorld> Director;
    EFDKind Kind = EFDKind::Decoration;
    int32 Room = 0;
    FString ObjectId;
    FVector Size = FVector(100);
    FVector InitialLocation = FVector::ZeroVector;
    FVector SlideTarget = FVector::ZeroVector;
    bool bMoved = false;
    bool bSliding = false;
    bool bBurning = false;
    bool bBurned = false;
    bool bLit = false;
    bool bProjected = false;
    int32 RequiredView = INDEX_NONE;
    bool bActivated = false;
    float BurnTime = 0;
    float LitRemaining = 0;
    void Configure(AFDWorld* World, EFDKind Type, int32 RoomId, const FString& Id, FVector Location, FVector Dimensions, FLinearColor Color);
    void SetProjection(bool Enabled, float PlaneDepth, int32 View = 0);
    void Ignite();
    void ResetObject();
    void Slide();
    bool IsWalkable() const;
    bool Blocks() const;
    FVector GetExtent() const { return Size * .5f; }
    float Top() const { return GetActorLocation().Z + Size.Z * .5f; }
};

UCLASS()
class NEWMYPROJECT_API AFDCharacter : public ACharacter
{
    GENERATED_BODY()
public:
    AFDCharacter();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* Input) override;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USpringArmComponent> Boom;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UCameraComponent> Camera;
    UPROPERTY(VisibleAnywhere) TObjectPtr<USceneComponent> WaxBody;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Flame;
    UPROPERTY() TObjectPtr<UPointLightComponent> CandleLight;
    UPROPERTY() TObjectPtr<AFDWorld> Director;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> BodyPieces;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> WalkingFeet;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> CastingHands;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> BlinkingEyes;
    UPROPERTY() TObjectPtr<UStaticMeshComponent> Cape;
    float VisualTime = 0;
    float CastPose = 0;
    int32 CastSerial = 0;
    bool bSkeletalHero = false;
    float LandingPulse = 0;
    bool bVisualAirborne = false;
    bool b2D = false;
    bool bPreview = false;
    bool bTransition = false;
    float PlaneDepth = 0;
    int32 FoldView = 0;
    float TransitionTime = 0;
    float JumpBuffer = 0;
    float CoyoteTime = 0;
    void MoveForward(float Value);
    void MoveRight(float Value);
    void Turn(float Value);
    void LookUp(float Value);
    void JumpPressed();
    void PreviewStart();
    void PreviewCommit();
    void PreviousView();
    void NextView();
    bool SelectView(int32 View);
    void UpdateViewCamera();
    bool TrySwitch();
    bool CanSwitch(FVector& Destination, FString& Reason) const;
    void ApplyDimension(bool Is2D, FVector Destination);
    void CastFire();
    void Interact();
    void ResetRoom();
    void Hint();
    void PauseToggle();
    void Confirm();
    void NewGame();
    void Quit();
    bool InputAllowed() const;
};

UCLASS()
class NEWMYPROJECT_API AFDWorld : public AActor
{
    GENERATED_BODY()
public:
    AFDWorld();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY() TArray<TObjectPtr<AFDObject>> Objects;
    UPROPERTY() TObjectPtr<AFDCharacter> Player;
    UPROPERTY() TObjectPtr<USceneComponent> FireRoot;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> FirePieces;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> FireSigils;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> FireGlyphs;
    UPROPERTY() TObjectPtr<UNiagaraComponent> FireFX;
    UPROPERTY() TObjectPtr<UPointLightComponent> FireLight;
    UPROPERTY() TObjectPtr<USceneComponent> PreviewRoot;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> PreviewPieces;
    UPROPERTY() TObjectPtr<UFDSave> Save;
    UPROPERTY() TArray<TObjectPtr<USceneComponent>> SceneryRoots;
    int32 CurrentRoom = 0;
    int32 HintLevel = 0;
    bool bStarted = false;
    bool bPaused = false;
    bool bFinished = false;
    bool bInitialized = false;
    bool bUsed2D = false;
    bool bFurnaceSolved = false;
    float FurnaceHold = 0;
    float FireRemaining = 0;
    float CastCooldown = 0;
    float ToastRemaining = 0;
    float SessionTime = 0;
    int32 Falls = 0;
    int32 Switches = 0;
    FString Toast;
    FVector FireAnchor = FVector::ZeroVector;
    FVector FireSourceAnchor = FVector::ZeroVector;
    bool bSmoke = false;
    bool bCapture = false;
    bool bFourTest = false;
    int32 SmokeStep = 0;
    int32 SmokeFailures = 0;
    float SmokeClock = 0;
    void BuildWorld();
    AFDObject* Add(EFDKind Type, int32 Room, const FString& Id, FVector Local, FVector Size, FLinearColor Color);
    AFDObject* Find(const FString& Id) const;
    static FVector RoomOrigin(int32 Index) { return FVector(Index * 5000.0f, 0, 0); }
    void Start(bool Fresh);
    void EnterRoom(int32 Index, bool SaveProgress);
    void ResetCurrentRoom(bool FromFall = false);
    void Project(bool Is2D, float PlaneDepth);
    bool CheckSwitch(const AFDCharacter* Character, FVector& Destination, FString& Reason) const;
    bool IsSafe(FVector Location, const AFDCharacter* Character, bool Target2D, const AFDObject* Support = nullptr) const;
    void PlaceFire();
    FVector GetCastAnchor() const;
    void RefreshFireVisual();
    void AnimateFireVisual();
    void UpdateFire(float DeltaSeconds);
    void UseNearby();
    bool RoomSolved() const;
    void Notify(const FString& Message, float Duration = 3);
    void RequestHint();
    void Persist();
    FString RoomTitle() const;
    FString Objective() const;
    FString ContextAction() const;
    void RunSmoke(float DeltaSeconds);
    void RunTraversal(float DeltaSeconds);
    void RunFourViewTests(float DeltaSeconds);
    void UpdateScenery();
    void Check(bool Condition, const FString& Label);
};

UCLASS()
class NEWMYPROJECT_API AFDHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
    UPROPERTY(Transient) TObjectPtr<class UFont> ChineseFont;
    void Text(const FString& Value, float X, float Y, float Size = 1, FLinearColor Color = FLinearColor::White, bool Centered = false);
    void Panel(float X, float Y, float W, float H, float Alpha = .8f);
};

UCLASS()
class NEWMYPROJECT_API AFDGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    AFDGameMode();
    virtual void StartPlay() override;
};
