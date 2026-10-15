#include "FoldlightDemo.h"
#include "FDProjection.h"
#include "FDVisuals.h"
#include "FDMageAnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

AFDCharacter::AFDCharacter()
{
    PrimaryActorTick.bCanEverTick=true;
    GetCapsuleComponent()->InitCapsuleSize(30,86);
    bUseControllerRotationYaw=false;
    GetCharacterMovement()->bOrientRotationToMovement=true;
    GetCharacterMovement()->RotationRate=FRotator(0,800,0);
    GetCharacterMovement()->MaxWalkSpeed=420;
    GetCharacterMovement()->JumpZVelocity=540;
    GetCharacterMovement()->GravityScale=1.5f;
    GetCharacterMovement()->AirControl=.75f;
    GetCharacterMovement()->BrakingDecelerationWalking=2000;
    GetCharacterMovement()->MaxStepHeight=32;
    Boom=CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    Boom->SetupAttachment(RootComponent);
    Boom->TargetArmLength=720;
    Boom->bUsePawnControlRotation=true;
    Boom->bDoCollisionTest=true;
    Boom->TargetOffset=FVector(0,0,45);
    Camera=CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(Boom,USpringArmComponent::SocketName);
    Camera->FieldOfView=65;
    Camera->OrthoWidth=1400;
    Camera->PostProcessSettings.bOverride_AutoExposureMethod=true;
    Camera->PostProcessSettings.AutoExposureMethod=EAutoExposureMethod::AEM_Manual;
    Camera->PostProcessSettings.bOverride_AutoExposureBias=true;
    Camera->PostProcessSettings.AutoExposureBias=9.5f;
    Camera->PostProcessSettings.bOverride_MotionBlurAmount=true;
    Camera->PostProcessSettings.MotionBlurAmount=0;
    WaxBody=CreateDefaultSubobject<USceneComponent>(TEXT("WaxBody"));
    WaxBody->SetupAttachment(RootComponent);
    WaxBody->SetRelativeLocation(FVector(0,0,-86));
    CandleLight=CreateDefaultSubobject<UPointLightComponent>(TEXT("CandleLight"));
    CandleLight->SetupAttachment(WaxBody);
    CandleLight->SetRelativeLocation(FVector(0,0,137));
    CandleLight->SetLightColor(FLinearColor(1,.58f,.23f));
    CandleLight->SetIntensity(90);
    CandleLight->SetAttenuationRadius(230);
    CandleLight->SetCastShadows(false);
}

void AFDCharacter::BeginPlay()
{
    Super::BeginPlay();
    if (!FParse::Param(FCommandLine::Get(),TEXT("CandleHero")))
    {
        if (USkeletalMesh* Mage=LoadObject<USkeletalMesh>(nullptr,TEXT("/Game/Foldlight/External/KayKit/Mage/Mage/SkeletalMeshes/Mage.Mage")))
        {
            bSkeletalHero=true;
            GetMesh()->SetSkeletalMeshAsset(Mage);
            GetMesh()->SetRelativeLocation(FVector(0,0,-86));
            GetMesh()->SetRelativeRotation(FRotator(0,-90,0));
            GetMesh()->SetRelativeScale3D(FVector(.65f));
            GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            GetMesh()->VisibilityBasedAnimTickOption=EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
            GetMesh()->SetAnimInstanceClass(UFDMageAnimInstance::StaticClass());
            CandleLight->SetRelativeLocation(FVector(15,0,120));
            GetCharacterMovement()->SetComponentTickEnabled(false);
            return;
        }
    }
    const FLinearColor Wax(.91f,.80f,.57f), Dark(.035f,.095f,.12f);
    BodyPieces.Add(FDVisuals::Piece(this,WaxBody,TEXT("SM_Wax"),FVector(0,0,79),FVector(50,50,82),Wax));
    Cape=FDVisuals::Piece(this,WaxBody,TEXT("SM_Cape"),FVector(-3,0,42),FVector(76,74,67),Dark);
    BodyPieces.Add(Cape);
    BodyPieces.Add(FDVisuals::Piece(this,WaxBody,TEXT("SM_Ring"),FVector(0,0,70),FVector(41,41,5),FDVisuals::Gold));
    BodyPieces.Add(FDVisuals::Piece(this,WaxBody,TEXT("SM_Crystal"),FVector(23,0,64),FVector(8,12,17),FDVisuals::Gold,.5f));
    // Melted rim and individual wax runs give the candle a recognizable silhouette.
    for (int I=0;I<8;++I)
    {
        const float A=I*PI*.25f, Length=10+(I*13)%22;
        BodyPieces.Add(FDVisuals::Piece(this,WaxBody,TEXT("Sphere"),FVector(FMath::Cos(A)*23,FMath::Sin(A)*23,115-Length*.4f),FVector(7,7,Length),Wax));
    }
    for (float Y : {-11.f,11.f})
    {
        UStaticMeshComponent* Eye=FDVisuals::Piece(this,WaxBody,TEXT("Sphere"),FVector(23,Y,95),FVector(6,7,12),Dark);
        BodyPieces.Add(Eye); BlinkingEyes.Add(Eye);
    }
    for (float Y : {-23.f,23.f})
    {
        UStaticMeshComponent* Foot=FDVisuals::Piece(this,WaxBody,TEXT("Sphere"),FVector(8,Y,12),FVector(32,22,23),Dark);
        UStaticMeshComponent* Hand=FDVisuals::Piece(this,WaxBody,TEXT("Sphere"),FVector(0,Y*1.4f,59),FVector(18,18,25),Wax);
        BodyPieces.Add(Foot); WalkingFeet.Add(Foot);
        BodyPieces.Add(Hand); CastingHands.Add(Hand);
    }
    BodyPieces.Add(FDVisuals::Piece(this,WaxBody,TEXT("Cylinder"),FVector(0,0,124),FVector(6,6,16),Dark));
    Flame=FDVisuals::Piece(this,WaxBody,TEXT("SM_Flame"),FVector(0,0,142),FVector(21,21,40),FDVisuals::Gold,5);
    BodyPieces.Add(Flame);
    GetCharacterMovement()->SetComponentTickEnabled(false);
}

void AFDCharacter::SetupPlayerInputComponent(UInputComponent* Input)
{
    Super::SetupPlayerInputComponent(Input);
    Input->BindAxis(TEXT("FD_Forward"),this,&AFDCharacter::MoveForward);
    Input->BindAxis(TEXT("FD_Right"),this,&AFDCharacter::MoveRight);
    Input->BindAxis(TEXT("FD_Turn"),this,&AFDCharacter::Turn);
    Input->BindAxis(TEXT("FD_Look"),this,&AFDCharacter::LookUp);
    Input->BindKey(EKeys::SpaceBar,IE_Pressed,this,&AFDCharacter::JumpPressed);
    Input->BindKey(EKeys::SpaceBar,IE_Released,this,&ACharacter::StopJumping);
    Input->BindKey(EKeys::V,IE_Pressed,this,&AFDCharacter::PreviewStart);
    Input->BindKey(EKeys::V,IE_Released,this,&AFDCharacter::PreviewCommit);
    Input->BindKey(EKeys::Q,IE_Pressed,this,&AFDCharacter::PreviousView);
    Input->BindKey(EKeys::F,IE_Pressed,this,&AFDCharacter::NextView);
    Input->BindKey(EKeys::G,IE_Pressed,this,&AFDCharacter::CastFire);
    Input->BindKey(EKeys::E,IE_Pressed,this,&AFDCharacter::Interact);
    Input->BindKey(EKeys::R,IE_Pressed,this,&AFDCharacter::ResetRoom);
    Input->BindKey(EKeys::H,IE_Pressed,this,&AFDCharacter::Hint);
    Input->BindKey(EKeys::Escape,IE_Pressed,this,&AFDCharacter::PauseToggle);
    Input->BindKey(EKeys::Enter,IE_Pressed,this,&AFDCharacter::Confirm);
    Input->BindKey(EKeys::N,IE_Pressed,this,&AFDCharacter::NewGame);
    Input->BindKey(EKeys::F10,IE_Pressed,this,&AFDCharacter::Quit);
}

bool AFDCharacter::InputAllowed() const { return Director && Director->bStarted && !Director->bPaused && !Director->bFinished && !bTransition; }
void AFDCharacter::MoveForward(float Value)
{
    if (!InputAllowed() || b2D || bPreview || !Controller) return;
    AddMovementInput(FRotationMatrix(FRotator(0,Controller->GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::X),Value);
}
void AFDCharacter::MoveRight(float Value)
{
    if (!InputAllowed() || bPreview || !Controller) return;
    FVector Right=b2D ? FDProjection::Right(FoldView) : FRotationMatrix(FRotator(0,Controller->GetControlRotation().Yaw,0)).GetUnitAxis(EAxis::Y);
    AddMovementInput(Right,Value);
}
void AFDCharacter::Turn(float Value) { if (InputAllowed() && !b2D && !bPreview) AddControllerYawInput(Value); }
void AFDCharacter::LookUp(float Value) { if (InputAllowed() && !b2D && !bPreview) AddControllerPitchInput(Value); }
void AFDCharacter::JumpPressed() { if (InputAllowed() && !bPreview) JumpBuffer=.14f; }
void AFDCharacter::PreviewStart()
{
    if (!InputAllowed()) return;
    bPreview=true;
    GetCharacterMovement()->StopMovementImmediately();
    if (!b2D)
    {
        Boom->bUsePawnControlRotation=false;
        Boom->bDoCollisionTest=false;
        Boom->SetAbsolute(false,true,false);
        Boom->SetWorldRotation(FRotator(0,FDProjection::Yaw(FoldView),0));
        Camera->SetProjectionMode(ECameraProjectionMode::Orthographic);
        Camera->SetOrthoWidth(1400);
    }
}
void AFDCharacter::PreviewCommit()
{
    if (!bPreview) return;
    bPreview=false;
    if (!b2D)
    {
        Boom->SetAbsolute(false,false,false);
        Boom->bUsePawnControlRotation=true;
        Boom->bDoCollisionTest=true;
        Camera->SetProjectionMode(ECameraProjectionMode::Perspective);
    }
    TrySwitch();
}
bool AFDCharacter::CanSwitch(FVector& Destination, FString& Reason) const
{
    if (!Director) { Reason=TEXT("房间正在加载，请稍候。"); return false; }
    return Director->CheckSwitch(this,Destination,Reason);
}
void AFDCharacter::PreviousView() { SelectView(FoldView-1); }
void AFDCharacter::NextView() { SelectView(FoldView+1); }
bool AFDCharacter::SelectView(int32 View)
{
    if (!InputAllowed()) return false;
    if (b2D) { Director->Notify(TEXT("先按 V 展开为三维，再用 Q / F 选择新的折叠方向。")); return false; }
    FoldView=FDProjection::Wrap(View);
    UpdateViewCamera();
    Director->UpdateScenery();
    Director->Notify(FString::Printf(TEXT("已选择%s。按住 V 预览；预览中也可用 Q / F 换方向。"),FDProjection::Name(FoldView)));
    return true;
}
void AFDCharacter::UpdateViewCamera()
{
    if (b2D || bPreview)
    {
        Boom->bUsePawnControlRotation=false; Boom->bDoCollisionTest=false;
        Boom->SetAbsolute(false,true,false);
        Boom->SetWorldRotation(FRotator(0,FDProjection::Yaw(FoldView),0));
        Camera->SetProjectionMode(ECameraProjectionMode::Orthographic);
        Camera->SetOrthoWidth(1400);
    }
    else if (Controller) Controller->SetControlRotation(FRotator(-27,FDProjection::Yaw(FoldView),0));
}
bool AFDCharacter::TrySwitch()
{
    if (!InputAllowed()) return false;
    FVector Destination; FString Reason;
    if (!CanSwitch(Destination,Reason)) { Director->Notify(Reason); return false; }
    ApplyDimension(!b2D,Destination);
    FDVisuals::Sound(this,TEXT("S_Fold"),.4f);
    Director->Switches++;
    if (b2D) Director->bUsed2D=true;
    return true;
}
void AFDCharacter::ApplyDimension(bool Is2D, FVector Destination)
{
    b2D=Is2D; bPreview=false;
    GetCharacterMovement()->StopMovementImmediately();
    if (Is2D)
    {
        PlaneDepth=FVector::DotProduct(Destination,FDProjection::Depth(FoldView));
        Director->Project(true,PlaneDepth);
        GetCharacterMovement()->SetPlaneConstraintNormal(FDProjection::Depth(FoldView));
        GetCharacterMovement()->SetPlaneConstraintOrigin(Destination);
        GetCharacterMovement()->SetPlaneConstraintEnabled(true);
        GetCharacterMovement()->RotationRate=FRotator(0,10000,0);
        const FVector Right=FDProjection::Right(FoldView);
        const float Facing=FVector::DotProduct(GetActorForwardVector(),Right)<0 ? -1.f : 1.f;
        SetActorRotation((Right*Facing).Rotation());
        Boom->bUsePawnControlRotation=false;
        Boom->bDoCollisionTest=false;
        Boom->SetAbsolute(false,true,false);
        Boom->SetWorldRotation(FRotator(0,FDProjection::Yaw(FoldView),0));
        Camera->SetProjectionMode(ECameraProjectionMode::Orthographic);
        Camera->SetOrthoWidth(1400);
    }
    else
    {
        GetCharacterMovement()->SetPlaneConstraintEnabled(false);
        Director->Project(false,0);
        GetCharacterMovement()->RotationRate=FRotator(0,800,0);
        Boom->SetAbsolute(false,false,false);
        Boom->bUsePawnControlRotation=true;
        Boom->bDoCollisionTest=true;
        if (Controller) Controller->SetControlRotation(FRotator(-27,FDProjection::Yaw(FoldView),0));
        Camera->SetProjectionMode(ECameraProjectionMode::Perspective);
    }
    SetActorLocation(Destination,false,nullptr,ETeleportType::TeleportPhysics);
    GetCharacterMovement()->SetMovementMode(MOVE_Falling);
    bTransition=true; TransitionTime=.3f;
    Director->RefreshFireVisual();
    Director->UpdateScenery();
}

void AFDCharacter::CastFire()
{
    if (InputAllowed() && !bPreview)
    {
        Director->PlaceFire();
    }
}
void AFDCharacter::Interact() { if (InputAllowed() && !bPreview) Director->UseNearby(); }
void AFDCharacter::ResetRoom() { if (InputAllowed()) Director->ResetCurrentRoom(); }
void AFDCharacter::Hint() { if (InputAllowed()) Director->RequestHint(); }
void AFDCharacter::PauseToggle()
{
    if (!Director || !Director->bStarted || Director->bFinished) return;
    Director->bPaused=!Director->bPaused;
    GetCharacterMovement()->SetComponentTickEnabled(!Director->bPaused);
    if (bPreview && !b2D)
    {
        Boom->SetAbsolute(false,false,false);
        Boom->bUsePawnControlRotation=true;
        Boom->bDoCollisionTest=true;
        Camera->SetProjectionMode(ECameraProjectionMode::Perspective);
    }
    bPreview=false;
}
void AFDCharacter::Confirm()
{
    if (!Director) return;
    if (!Director->bStarted) Director->Start(false);
    else if (Director->bPaused) PauseToggle();
    else if (Director->bFinished) Director->Start(true);
}
void AFDCharacter::NewGame() { if (Director && (!Director->bStarted || Director->bFinished)) Director->Start(true); }
void AFDCharacter::Quit() { UKismetSystemLibrary::QuitGame(this,Cast<APlayerController>(Controller),EQuitPreference::Quit,false); }

void AFDCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!Director) Director=Cast<AFDWorld>(UGameplayStatics::GetActorOfClass(this,AFDWorld::StaticClass()));
    if (bSkeletalHero) GetMesh()->bPauseAnims=Director && Director->bPaused;
    if (!Director || Director->bPaused) return;
    if (Controller && Cast<APlayerController>(Controller))
        Cast<APlayerController>(Controller)->PlayerCameraManager->ViewPitchMin=-65;
    if (Controller && Cast<APlayerController>(Controller))
        Cast<APlayerController>(Controller)->PlayerCameraManager->ViewPitchMax=10;
    if (bTransition)
    {
        TransitionTime-=DeltaSeconds;
        if (TransitionTime<=0) bTransition=false;
    }
    Boom->TargetArmLength=FMath::FInterpTo(Boom->TargetArmLength,(b2D || bPreview) ? 1600.f : 720.f,DeltaSeconds,12);
    if (InputAllowed())
    {
        JumpBuffer=FMath::Max(0.f,JumpBuffer-DeltaSeconds);
        CoyoteTime=GetCharacterMovement()->IsMovingOnGround() ? .1f : FMath::Max(0.f,CoyoteTime-DeltaSeconds);
        if (JumpBuffer>0 && (GetCharacterMovement()->IsMovingOnGround() || CoyoteTime>0))
        {
            LaunchCharacter(FVector(0,0,540),false,true);
            JumpBuffer=CoyoteTime=0;
        }
    }
    const float Speed=GetVelocity().Size2D();
    VisualTime+=DeltaSeconds;
    const float Time=VisualTime, Walk=FMath::Clamp(Speed/420.f,0.f,1.f);
    const bool Airborne=GetCharacterMovement()->IsFalling();
    if (bVisualAirborne && !Airborne) LandingPulse=1;
    bVisualAirborne=Airborne;
    LandingPulse=FMath::Max(0.f,LandingPulse-DeltaSeconds*5);
    CastPose=FMath::Max(0.f,CastPose-DeltaSeconds);
    if (bSkeletalHero)
    {
        CandleLight->SetIntensity(65+CastPose*65);
        return;
    }
    const float Cast=FMath::Sin(FMath::Clamp(CastPose/.55f,0.f,1.f)*PI);
    WaxBody->SetRelativeLocation(FVector(0,0,-86+FMath::Sin(Time*12)*Walk*2+FMath::Sin(Time*2)*.45f));
    WaxBody->SetRelativeRotation(FRotator(-Walk*3-Cast*4,0,FMath::Sin(Time*6)*Walk*2));
    WaxBody->SetRelativeScale3D(FVector(1+LandingPulse*.045f,1+LandingPulse*.045f,1-LandingPulse*.07f));
    for (int I=0;I<WalkingFeet.Num();++I)
    {
        const float Side=I ? 1.f : -1.f, Stride=FMath::Sin(Time*12+I*PI);
        WalkingFeet[I]->SetRelativeLocation(FVector(8+Stride*Walk*12,Side*23,12+FMath::Max(0.f,Stride)*Walk*8+(Airborne ? 5.f : 0.f)));
        CastingHands[I]->SetRelativeLocation(FVector(-Stride*Walk*7+Cast*28,Side*(32-Cast*9),59+Cast*18));
    }
    const float BlinkPhase=FMath::Fmod(Time,4.3f);
    const float EyeOpen=BlinkPhase<.16f ? FMath::Clamp(FMath::Abs(BlinkPhase-.08f)/.08f,.08f,1.f) : 1.f;
    for (UStaticMeshComponent* Eye : BlinkingEyes) Eye->SetRelativeScale3D(FVector(.06f,.07f,.12f*EyeOpen));
    if (Cape) Cape->SetRelativeRotation(FRotator(Walk*5+FMath::Sin(Time*3)*1.5f,0,FMath::Sin(Time*5)*Walk*3));
    if (Flame)
    {
        Flame->SetRelativeScale3D(FVector(.21f,.21f,.4f+FMath::Sin(Time*7)*.035f+Cast*.08f));
        Flame->SetRelativeRotation(FRotator(FMath::Sin(Time*4)*4-Walk*9,0,FMath::Sin(Time*3)*3));
    }
    CandleLight->SetIntensity(100+FMath::Sin(Time*8)*8+Cast*35);
}
