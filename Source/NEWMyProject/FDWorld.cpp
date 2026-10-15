#include "FoldlightDemo.h"
#include "FDProjection.h"
#include "FDVisuals.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

AFDWorld::AFDWorld()
{
    PrimaryActorTick.bCanEverTick=true;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
    FireRoot=CreateDefaultSubobject<USceneComponent>(TEXT("FireRoot"));
    FireRoot->SetupAttachment(RootComponent);
    FireLight=CreateDefaultSubobject<UPointLightComponent>(TEXT("FireLight"));
    FireLight->SetupAttachment(FireRoot);
    FireLight->SetIntensity(600);
    FireLight->SetAttenuationRadius(650);
    FireLight->SetLightColor(FDVisuals::Gold);
    FireLight->SetCastShadows(false);
    FireFX=CreateDefaultSubobject<UNiagaraComponent>(TEXT("FireFX"));
    FireFX->SetupAttachment(FireRoot);
    FireFX->SetAutoActivate(false);
    PreviewRoot=CreateDefaultSubobject<USceneComponent>(TEXT("Preview"));
    PreviewRoot->SetupAttachment(RootComponent);
}

void AFDWorld::BeginPlay()
{
    Super::BeginPlay();
    bSmoke=FParse::Param(FCommandLine::Get(),TEXT("FoldlightSmoke")) || FParse::Param(FCommandLine::Get(),TEXT("FoldlightTraversal"));
    bCapture=FParse::Param(FCommandLine::Get(),TEXT("FoldlightCapture"));
    BuildWorld();
    if (UGameplayStatics::DoesSaveGameExist(TEXT("FoldlightTrial_v1"),0))
        Save=Cast<UFDSave>(UGameplayStatics::LoadGameFromSlot(TEXT("FoldlightTrial_v1"),0));
    if (!Save || Save->Version!=1) Save=Cast<UFDSave>(UGameplayStatics::CreateSaveGameObject(UFDSave::StaticClass()));
    CurrentRoom=FMath::Clamp(Save->Room,0,4);
}

AFDObject* AFDWorld::Add(EFDKind Type, int32 Room, const FString& Id, FVector Local, FVector Size, FLinearColor Color)
{
    AFDObject* O=GetWorld()->SpawnActor<AFDObject>();
    O->Configure(this,Type,Room,Id,RoomOrigin(Room)+Local,Size,Color);
    Objects.Add(O); return O;
}
AFDObject* AFDWorld::Find(const FString& Id) const
{
    for (AFDObject* O : Objects) if (O && O->ObjectId==Id) return O;
    return nullptr;
}

void AFDWorld::BuildWorld()
{
    using namespace FDVisuals;
    const auto Platform=[&](int32 R,const TCHAR* Id,float X,float Y,float Top,float SX,float SY)
    {
        const float Height=Top+180;
        return Add(EFDKind::Platform,R,Id,FVector(X,Y,Top-Height*.5f),FVector(SX,SY,Height),Stone);
    };
    const auto Pillar=[&](int32 R,const TCHAR* Id,float X,float Y,float Top,float TargetX)
    {
        AFDObject* O=Add(EFDKind::Pillar,R,Id,FVector(X,Y,(Top-180)*.5f),FVector(320,340,Top+180),FLinearColor(.18f,.30f,.30f));
        O->SlideTarget=O->InitialLocation+FVector(TargetX-X,0,0);
        Piece(this,RootComponent,TEXT("Cube"),RoomOrigin(R)+FVector((X+TargetX)*.5f,Y,-174),FVector(FMath::Abs(TargetX-X)+320,18,10),Gold,1);
        for (float DY : {-170.f,170.f})
            Piece(this,RootComponent,TEXT("Cube"),RoomOrigin(R)+FVector(TargetX,Y+DY,-166),FVector(320,8,8),Gold,2);
        return O;
    };
    const auto Exit=[&](int32 R,float X,float Y,float Z)
    { Add(EFDKind::Exit,R,FString::Printf(TEXT("exit%d"),R),FVector(X,Y,Z+8),FVector(180,180,10),Gold); };

    Platform(0,TEXT("r0_start"),-950,0,0,900,1100);
    Platform(0,TEXT("r0_a"),-300,-500,70,420,350);
    Platform(0,TEXT("r0_b"),100,500,140,380,350);
    Platform(0,TEXT("r0_c"),490,-450,210,400,350);
    Platform(0,TEXT("r0_goal"),900,550,280,450,450); Exit(0,940,550,280);

    Platform(1,TEXT("r1_start"),-950,0,0,900,1200);
    Platform(1,TEXT("r1_a"),-320,450,70,360,350);
    Pillar(1,TEXT("r1_pillar"),-730,-450,140,80);
    Platform(1,TEXT("r1_c"),500,450,210,400,350);
    Platform(1,TEXT("r1_goal"),900,550,280,450,450); Exit(1,940,550,280);

    Platform(2,TEXT("r2_start"),0,0,0,2650,1100);
    Add(EFDKind::Vine,2,TEXT("r2_vine"),FVector(100,0,190),FVector(90,1060,380),Gold);
    Exit(2,1030,0,0);

    Platform(3,TEXT("r3_start"),-250,0,0,2300,1200);
    Add(EFDKind::Receiver,3,TEXT("r3_left"),FVector(-650,0,38),FVector(60,60,76),Gold);
    Add(EFDKind::Receiver,3,TEXT("r3_right"),FVector(-350,0,38),FVector(60,60,76),Gold);
    Add(EFDKind::Vine,3,TEXT("r3_vine"),FVector(450,450,410),FVector(80,350,380),Gold);
    Platform(3,TEXT("r3_step"),780,-430,70,280,350);
    Platform(3,TEXT("r3_goal"),1080,450,140,350,450); Exit(3,1110,450,140);
    // A suspended root seal communicates the second lock of the furnace room.
    Piece(this,RootComponent,TEXT("Cube"),RoomOrigin(3)+FVector(450,450,615),FVector(140,450,25),Stone);

    // Four segments turn south -> west -> north -> east. Floating slabs avoid
    // making later, higher platforms solid columns across earlier projections.
    const auto Slab=[&](const TCHAR* Id,FVector Center,float Top,FVector2D Footprint)
    { return Add(EFDKind::Platform,4,Id,FVector(Center.X,Center.Y,Top-30),FVector(Footprint.X,Footprint.Y,60),Stone); };
    Platform(4,TEXT("r4_start"),-1050,0,0,800,1100);
    Pillar(4,TEXT("r4_pillar"),-790,-450,70,-450);
    Slab(TEXT("r4_b"),FVector(0,500,0),140,FVector2D(400,400));
    AFDObject* South=Slab(TEXT("r4_south"),FVector(450,-450,0),210,FVector2D(450,450));
    Slab(TEXT("r4_w1"),FVector(-1550,-30,0),280,FVector2D(350,380));
    AFDObject* West=Slab(TEXT("r4_west"),FVector(-1050,390,0),350,FVector2D(450,450));
    Slab(TEXT("r4_n1"),FVector(-1500,900,0),420,FVector2D(380,350));
    AFDObject* North=Slab(TEXT("r4_north"),FVector(-1950,-900,0),490,FVector2D(450,450));
    Slab(TEXT("r4_e1"),FVector(0,-1350,0),560,FVector2D(350,380));
    AFDObject* East=Slab(TEXT("r4_goal"),FVector(500,-1800,0),630,FVector2D(450,450));
    const TArray<AFDObject*> Landings={South,East,North,West};
    for (int32 View=0;View<4;++View)
    {
        AFDObject* Landing=Landings[View];
        const FVector Local=Landing->GetActorLocation()-RoomOrigin(4)+FDProjection::Right(View)*230+FVector(0,0,150);
        AFDObject* Seal=Add(EFDKind::Receiver,4,FString::Printf(TEXT("r4_seal%d"),View),Local,FVector(70,70,90),Gold);
        Seal->RequiredView=View;
        // Four bars form a directional arrow on the landing.
        const FVector R=FDProjection::Right(View), N=FDProjection::Depth(View);
        for (int32 I=0;I<4;++I)
        {
            FVector P=Landing->GetActorLocation()+R*(I*35-55); P.Z=Landing->Top()+2;
            UStaticMeshComponent* Mark=Piece(this,Landing->VisualRoot,TEXT("Cube"),FVector::ZeroVector,FVector(7,100-I*22,3),Gold,.8f);
            Mark->SetWorldLocation(P); Mark->SetWorldRotation(R.Rotation());
        }
    }
    Exit(4,500,-1800,630);

    // Scenic architecture is intentionally collision-free and never participates in projection.
    for (int32 R=0;R<5;++R)
    {
        FVector O=RoomOrigin(R);
        USceneComponent* Scenery=NewObject<USceneComponent>(this);
        AddInstanceComponent(Scenery); Scenery->SetupAttachment(RootComponent);
        Scenery->SetRelativeLocation(O); Scenery->RegisterComponent(); SceneryRoots.Add(Scenery);
        BuildScenery(this,Scenery,R);
        for (float X : {-1050.f,1000.f})
        {
            UPointLightComponent* Light=NewObject<UPointLightComponent>(this);
            AddInstanceComponent(Light); Light->SetupAttachment(RootComponent);
            Light->SetMobility(EComponentMobility::Movable);
            Light->SetRelativeLocation(O+FVector(X,100,550));
            Light->SetIntensity(6500); Light->SetAttenuationRadius(2100);
            Light->SetLightColor(X<0 ? FLinearColor(.44f,.73f,.83f) : FLinearColor(1,.64f,.29f));
            Light->SetCastShadows(false); Light->RegisterComponent();
        }
    }
    ADirectionalLight* Sun=GetWorld()->SpawnActor<ADirectionalLight>(FVector::ZeroVector,FRotator(-48,-35,0));
    Sun->GetLightComponent()->SetIntensity(5.f);
    Cast<UDirectionalLightComponent>(Sun->GetLightComponent())->SetForwardShadingPriority(1);
    Sun->GetLightComponent()->SetLightColor(FLinearColor(.72f,.85f,1));
    ADirectionalLight* Fill=GetWorld()->SpawnActor<ADirectionalLight>(FVector::ZeroVector,FRotator(-20,-90,0));
    Fill->GetLightComponent()->SetMobility(EComponentMobility::Movable);
    Fill->GetLightComponent()->SetIntensity(2.7f);
    Fill->GetLightComponent()->SetLightColor(FLinearColor(.64f,.82f,.91f));
    Fill->GetLightComponent()->SetCastShadows(false);
    ASkyLight* Sky=GetWorld()->SpawnActor<ASkyLight>();
    Sky->GetLightComponent()->SetIntensity(.65f);
    Sky->GetLightComponent()->SetLightColor(FLinearColor(.30f,.48f,.60f));
    Sky->GetLightComponent()->SetCastShadows(false);
    Sky->GetLightComponent()->RecaptureSky();
    AExponentialHeightFog* Fog=GetWorld()->SpawnActor<AExponentialHeightFog>();
    Fog->GetComponent()->SetFogDensity(.014f);
    Fog->GetComponent()->SetFogInscatteringColor(FLinearColor(.035f,.07f,.095f));
    Fog->SetActorLocation(FVector(0,0,-300));
    UMaterialInterface* SpellSurface=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Foldlight/Art/M_FireFlow.M_FireFlow"));
    // Eighteen crossed flame pairs, twenty-four soft embers and six smoke wisps. Each has its
    // own phase; all animation uses spell age so folding never restarts it.
    for (int I=0;I<66;++I)
    {
        UMaterialInstanceDynamic* SpellMaterial=UMaterialInstanceDynamic::Create(SpellSurface,this);
        SpellMaterial->SetScalarParameterValue(TEXT("Seed"),(I/2)*.731f);
        SpellMaterial->SetScalarParameterValue(TEXT("Ember"),I>=36 && I<60 ? 1.f : 0.f);
        SpellMaterial->SetScalarParameterValue(TEXT("Smoke"),I>=60 ? 1.f : 0.f);
        UStaticMeshComponent* Wisp=Piece(this,FireRoot,TEXT("Plane"),FVector::ZeroVector,FVector(20),Gold);
        Wisp->SetMaterial(0,SpellMaterial); Wisp->SetCastShadow(false); FirePieces.Add(Wisp);
    }
    for (int I=0;I<2;++I)
    {
        UStaticMeshComponent* Ring=Piece(this,FireRoot,TEXT("SM_Sigil"),FVector::ZeroVector,FVector(100,100,1),FLinearColor(.85f,.32f,.07f),.6f);
        Ring->SetCastShadow(false); FireSigils.Add(Ring);
    }
    for (int I=0;I<12;++I)
    {
        UStaticMeshComponent* Glyph=Piece(this,FireRoot,TEXT("Cube"),FVector::ZeroVector,FVector(12,3,1),Gold,.7f);
        Glyph->SetCastShadow(false); FireGlyphs.Add(Glyph);
    }
    for (int I=0;I<4;++I)
        PreviewPieces.Add(Piece(this,PreviewRoot,TEXT("Cube"),FVector::ZeroVector,FVector(5),Edge,2));
    FireFX->SetAsset(LoadObject<UNiagaraSystem>(nullptr,TEXT("/Game/Vefects/Free_Fire/Shared/Particles/NS_Fire_Big_Simple.NS_Fire_Big_Simple")));
    FireRoot->SetVisibility(false,true); PreviewRoot->SetVisibility(false,true);
}

void AFDWorld::Start(bool Fresh)
{
    if (!Player || !Save) return;
    bStarted=true; bPaused=false; bFinished=false; Falls=Switches=0; SessionTime=0;
    if (Fresh || Save->Finished) { Save->Room=0; Save->Finished=false; Persist(); }
    EnterRoom(FMath::Clamp(Save->Room,0,4),false);
    Notify(TEXT("Q / F 选择南、东、北、西视角；按住 V 预览，松开切换。H 获取提示。"),8);
}

void AFDWorld::EnterRoom(int32 Index, bool SaveProgress)
{
    if (!Player) return;
    Project(false,0);
    CurrentRoom=FMath::Clamp(Index,0,4); HintLevel=0; bUsed2D=false;
    Player->FoldView=0;
    bFurnaceSolved=false; FurnaceHold=0; FireRemaining=0; CastCooldown=0;
    FireFX->DeactivateImmediate(); FireRoot->SetVisibility(false,true);
    for (AFDObject* O : Objects) if (O->Room==CurrentRoom) O->ResetObject();
    Player->GetCharacterMovement()->SetComponentTickEnabled(true);
    Player->ApplyDimension(false,RoomOrigin(CurrentRoom)+FVector(-1100,0,90));
    Player->SetActorRotation(FRotator(0,0,0));
    Player->JumpBuffer=Player->CoyoteTime=0;
    if (SaveProgress) { Save->Room=CurrentRoom; Save->Finished=false; Persist(); }
}

void AFDWorld::ResetCurrentRoom(bool FromFall)
{
    if (FromFall) Falls++;
    EnterRoom(CurrentRoom,false);
    Notify(FromFall ? TEXT("已返回本关检查点，机关已复位。") : TEXT("本关已重置，试试另一种排列方式。"));
}

void AFDWorld::Project(bool Is2D, float PlaneDepth)
{
    for (AFDObject* O : Objects)
    {
        O->SetProjection(Is2D && O->Room==CurrentRoom,PlaneDepth,Player ? Player->FoldView : 0);
        O->VisualRoot->SetVisibility(O->Room==CurrentRoom && !O->bBurned,true);
    }
    UpdateScenery();
}

void AFDWorld::UpdateScenery()
{
    if (!Player) return;
    for (AFDObject* O : Objects)
        if (O->Room==CurrentRoom && O->Kind==EFDKind::Exit)
            O->VisualRoot->SetWorldRotation(FDProjection::Right(Player->FoldView).Rotation());
    for (int32 I=0;I<SceneryRoots.Num();++I)
    {
        SceneryRoots[I]->SetVisibility(I==CurrentRoom,true);
        if (I!=CurrentRoom) continue;
        const FVector N=FDProjection::Depth(Player->FoldView);
        FVector Origin=RoomOrigin(I);
        float BackdropDepth=FVector::DotProduct(Player->GetActorLocation(),N);
        // The arcade's nearest ornaments are about 950 cm behind its origin.
        // Keep another 450 cm clear behind the farthest source platform in
        // this view, including the tall tower's opposite-side exit.
        for (AFDObject* Object : Objects)
            if (Object->Room==CurrentRoom)
                BackdropDepth=FMath::Min(BackdropDepth,FVector::DotProduct(Object->GetActorLocation(),N)-FVector::DotProduct(Object->GetExtent(),N.GetAbs())+500.f);
        Origin=FDProjection::OnPlane(Origin,Player->FoldView,BackdropDepth);
        SceneryRoots[I]->SetWorldLocation(Origin);
        SceneryRoots[I]->SetWorldRotation(FDProjection::Right(Player->FoldView).Rotation());
    }
}

bool AFDWorld::IsSafe(FVector Location,const AFDCharacter* Character,bool Target2D,const AFDObject* Support) const
{
    const float Radius=Character->GetCapsuleComponent()->GetScaledCapsuleRadius();
    const float Half=Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    for (AFDObject* O : Objects)
    {
        if (O->Room!=CurrentRoom || !O->Blocks() || O==Support) continue;
        FVector Center=O->GetActorLocation(), Ext=O->GetExtent();
        if (Target2D)
        {
            const int32 View=Character->FoldView;
            Center=FDProjection::OnPlane(Center,View,FVector::DotProduct(Location,FDProjection::Depth(View)));
            Ext=FDProjection::Right(View).GetAbs()*FDProjection::Width(O->Size,View)*.5f+FDProjection::Depth(View).GetAbs()*70+FVector(0,0,Ext.Z);
        }
        // Conservative capsule envelope keeps corners safe; touching a top is allowed.
        if (FMath::Abs(Location.X-Center.X)<Ext.X+Radius-2 &&
            FMath::Abs(Location.Y-Center.Y)<Ext.Y+Radius-2 &&
            FMath::Abs(Location.Z-Center.Z)<Ext.Z+Half-2) return false;
    }
    return true;
}

bool AFDWorld::CheckSwitch(const AFDCharacter* Character,FVector& Destination,FString& Reason) const
{
    Destination=Character->GetActorLocation();
    if (!Character->GetCharacterMovement()->IsMovingOnGround()) { Reason=TEXT("请先在平台上站稳，再切换维度。"); return false; }
    for (AFDObject* O : Objects) if (O->Room==CurrentRoom && O->bSliding) { Reason=TEXT("请等待石柱停止移动。"); return false; }
    if (!Character->b2D)
    {
        if (!IsSafe(Destination,Character,true)) { Reason=TEXT("折叠后会与障碍重叠，请移到空旷处。"); return false; }
        Reason=FString::Printf(TEXT("以%s折叠纵深。A / D 始终对应屏幕左 / 右；Q / F 可换预览方向。"),FDProjection::Name(Character->FoldView)); return true;
    }
    AFDObject* Support=Cast<AFDObject>(Character->GetCharacterMovement()->CurrentFloor.HitResult.GetActor());
    if (!Support || !Support->IsWalkable() || Support->Room!=CurrentRoom)
    { Reason=TEXT("请先落稳在带标记的平台上，再展开为三维。"); return false; }
    Destination=FDProjection::OnPlane(Destination,Character->FoldView,FVector::DotProduct(Support->GetActorLocation(),FDProjection::Depth(Character->FoldView)));
    Destination.Z=Support->Top()+Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()+2;
    if (FMath::Abs(FVector::DotProduct(Destination-Support->GetActorLocation(),FDProjection::Right(Character->FoldView)))>FDProjection::Width(Support->Size,Character->FoldView)*.5f-8)
    { Reason=TEXT("请远离平台边缘，确保安全展开。"); return false; }
    if (!IsSafe(Destination,Character,false,Support))
    { Reason=TEXT("三维落点被障碍占据，请换一个平台。"); return false; }
    Reason=TEXT("展开后，将回到脚下平台原本的三维位置。"); return true;
}

FVector AFDWorld::GetCastAnchor() const
{
    if (!Player) return FVector::ZeroVector;
    FVector Direction=Player->GetActorForwardVector();
    if (Player->b2D) { const FVector Right=FDProjection::Right(Player->FoldView); Direction=Right*(FVector::DotProduct(Direction,Right)<0 ? -1 : 1); }
    FVector Anchor=Player->GetActorLocation()+Direction*230;
    const float Feet=Player->GetActorLocation().Z-Player->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    Anchor.Z=Feet;
    float Best=-100000;
    for (AFDObject* O : Objects)
    {
        if (O->Room!=CurrentRoom || !O->IsWalkable()) continue;
        const FVector Delta=Anchor-O->GetActorLocation();
        if (Player->b2D)
        { if (FMath::Abs(FVector::DotProduct(Delta,FDProjection::Right(Player->FoldView)))>FDProjection::Width(O->Size,Player->FoldView)*.5f) continue; }
        else if (FMath::Abs(Delta.X)>O->Size.X*.5f || FMath::Abs(Delta.Y)>O->Size.Y*.5f) continue;
        if (O->Top()<=Feet+45 && O->Top()>Best) Best=O->Top();
    }
    if (Best>-10000) Anchor.Z=Best+3;
    return Anchor;
}

void AFDWorld::PlaceFire()
{
    if (CurrentRoom<2) { Notify(TEXT("火焰印记将在第三关「余烬花园」中解锁。")); return; }
    if (CastCooldown>0) return;
    if (!Player->GetCharacterMovement()->IsMovingOnGround()) { Notify(TEXT("请站稳在平台上，再施放火焰印记。")); return; }
    FireAnchor=GetCastAnchor();
    FireSourceAnchor=FireAnchor;
    if (Player->b2D)
    {
        // Use the support's real depth as the persistent anchor for a later unfold.
        if (AFDObject* Support=Cast<AFDObject>(Player->GetCharacterMovement()->CurrentFloor.HitResult.GetActor()))
            FireSourceAnchor=FDProjection::OnPlane(FireAnchor,Player->FoldView,FVector::DotProduct(Support->GetActorLocation(),FDProjection::Depth(Player->FoldView)));
    }
    FireRemaining=4; CastCooldown=.45f;
    Player->CastSerial++; Player->CastPose=.9f;
    FDVisuals::Sound(this,TEXT("S_Fire"),.5f);
    RefreshFireVisual();
}

void AFDWorld::RefreshFireVisual()
{
    if (!Player) return;
    FireRoot->SetVisibility(FireRemaining>0,true);
    if (FireRemaining<=0) return;
    FireAnchor=Player->b2D ? FDProjection::OnPlane(FireSourceAnchor,Player->FoldView,Player->PlaneDepth) : FireSourceAnchor;
    FireRoot->SetWorldLocation(FireAnchor);
    FireRoot->SetWorldRotation(Player->b2D ? FDProjection::Right(Player->FoldView).Rotation() : FRotator::ZeroRotator);
    FireLight->SetRelativeLocation(FVector(0,0,Player->b2D ? 150 : 65));
    FireLight->SetLightColor(FLinearColor(1,.38f,.055f));
    FireFX->DeactivateImmediate();
    AnimateFireVisual();
}

void AFDWorld::AnimateFireVisual()
{
    if (!Player || FireRemaining<=0) return;
    const bool Flat=Player->b2D;
    const float Time=4-FireRemaining;
    const float Fade=FMath::Clamp(FMath::Min(Time/.16f,FireRemaining/.55f),0.f,1.f);
    for (int I=0;I<FirePieces.Num();++I)
    {
        const bool Smoke=I>=60;
        const bool Ember=I>=36 && !Smoke;
        const int J=Smoke ? I-60 : Ember ? I-36 : I/2;
        const float A=J*2*PI/18;
        const float Phase=FMath::Fmod(Time*(.55f+J*.008f)+J*.137f,1.f);
        const float Flicker=.88f+.12f*FMath::Sin(Time*6+J*2.3f);
        FVector P,S;
        float Yaw=0;
        float Opacity=Fade;
        if (Ember || Smoke)
        {
            const float Drift=FMath::Sin(Time*2+J)*Phase*18;
            P=Flat ? FVector(FMath::Sin(J*2.4f)*43+Drift,-5,20+Phase*420)
                : FVector(FMath::Cos(A)*(174+Drift),FMath::Sin(A)*(174+Drift),14+Phase*125);
            S=Smoke ? FVector(Flat ? 80.f : 60.f,Flat ? 120.f : 75.f,1)*(.6f+Phase)
                : FVector(3.5f,7.f,1)*(1-Phase*.6f);
            if (Smoke)
                P=Flat ? FVector(FMath::Sin(J*2.4f)*26+Drift,8,230+Phase*200)
                    : FVector(FMath::Cos(J*PI/3)*(174+Drift),FMath::Sin(J*PI/3)*(174+Drift),55+Phase*100);
            Opacity*=FMath::Sin(Phase*PI);
            const FVector CameraLocal=FireRoot->GetComponentTransform().InverseTransformPosition(Player->Camera->GetComponentLocation());
            Yaw=(CameraLocal-P).Rotation().Yaw-90;
        }
        else
        {
            const float Height=(Flat ? (J<6 ? 285.f+90.f*(.5f+.5f*FMath::Sin(J*2.1f)) : 90.f+100.f*(.5f+.5f*FMath::Sin(J*2.1f))) : 58.f+30.f*(.5f+.5f*FMath::Sin(J*2.1f)))*Flicker;
            P=Flat ? FVector((J%3-1)*22,(J/3-2.5f)*5,Height*.5f+3)
                : FVector(FMath::Cos(A)*174,FMath::Sin(A)*174,Height*.5f+3);
            S=FVector(Flat ? (J<6 ? 85.f : 55.f) : 54.f,Height,1);
            if (Flat) Opacity*=J<6 ? .72f : .52f;
            Yaw=Flat ? (I%2)*70.f : FMath::RadiansToDegrees(A)+90+(I%2)*90;
            // Faster ignition expansion, then a quiet fade without collapsing
            // the flame tips into beads at the end of the spell.
            S.X*=.8f+.2f*FMath::Clamp(Time/.22f,0.f,1.f);
        }
        FirePieces[I]->SetRelativeLocation(P);
        FirePieces[I]->SetRelativeScale3D(S/100.f);
        FirePieces[I]->SetRelativeRotation(FRotator(0,Yaw,90));
        if (UMaterialInstanceDynamic* Mat=Cast<UMaterialInstanceDynamic>(FirePieces[I]->GetMaterial(0)))
        {
            Mat->SetScalarParameterValue(TEXT("Age"),Time);
            Mat->SetScalarParameterValue(TEXT("Fade"),Opacity);
        }
    }
    for (int I=0;I<FireSigils.Num();++I)
    {
        FireSigils[I]->SetRelativeLocation(Flat && I==0 ? FVector(0,0,220) : FVector(0,0,2+I));
        FireSigils[I]->SetRelativeScale3D((Flat ? (I==0 ? FVector(120,440,1.2f) : FVector(110,70,1)) : FVector(I==0 ? 400 : 335,I==0 ? 400 : 335,1))/100.f);
        FireSigils[I]->SetRelativeRotation(Flat && I==0 ? FRotator(0,0,90) : FRotator(0,Time*(I ? -9 : 6),0));
        if (UMaterialInstanceDynamic* Mat=Cast<UMaterialInstanceDynamic>(FireSigils[I]->GetMaterial(0)))
            Mat->SetVectorParameterValue(TEXT("Glow"),FLinearColor(1,.48f,.06f)*(Fade*1.8f));
    }
    for (int I=0;I<FireGlyphs.Num();++I)
    {
        const float A=I*PI/6+Time*.1f;
        FireGlyphs[I]->SetRelativeLocation(Flat ? FVector(I%2 ? 56 : -56,-2,35+(I/2)*73) : FVector(FMath::Cos(A)*184,FMath::Sin(A)*184,3));
        FireGlyphs[I]->SetRelativeScale3D(Flat ? FVector(.04f,.02f,.10f) : FVector(.14f,.025f,.01f));
        FireGlyphs[I]->SetRelativeRotation(Flat ? FRotator::ZeroRotator : FRotator(0,FMath::RadiansToDegrees(A),0));
        if (UMaterialInstanceDynamic* Mat=Cast<UMaterialInstanceDynamic>(FireGlyphs[I]->GetMaterial(0)))
            Mat->SetVectorParameterValue(TEXT("Glow"),FLinearColor(1,.48f,.06f)*(Fade*1.7f));
    }
    FireLight->SetIntensity((Flat ? 1800.f : 2500.f)*Fade*(.90f+.06f*FMath::Sin(Time*9)+.04f*FMath::Sin(Time*17)));
}

void AFDWorld::UpdateFire(float DeltaSeconds)
{
    if (FireRemaining<=0) return;
    FireRemaining=FMath::Max(0.f,FireRemaining-DeltaSeconds);
    if (FireRemaining<=0) { FireRoot->SetVisibility(false,true); FireFX->DeactivateImmediate(); return; }
    AnimateFireVisual();
    const bool Flat=Player->b2D;
    FVector Center=FireAnchor+FVector(0,0,Flat ? 220 : 35);
    for (AFDObject* O : Objects)
    {
        if (O->Room!=CurrentRoom || (O->Kind!=EFDKind::Vine && O->Kind!=EFDKind::Receiver)) continue;
        const FVector Delta=O->GetActorLocation()-Center, Ext=O->GetExtent();
        bool Hit=false;
        if (Flat)
            Hit=FMath::Abs(FVector::DotProduct(Delta,FDProjection::Right(Player->FoldView)))<60+FDProjection::Width(Ext,Player->FoldView) && FMath::Abs(Delta.Z)<220+Ext.Z;
        else
        {
            const float DX=FMath::Max(0.f,FMath::Abs(Delta.X)-Ext.X);
            const float DY=FMath::Max(0.f,FMath::Abs(Delta.Y)-Ext.Y);
            Hit=DX*DX+DY*DY<=200*200 && FMath::Abs(Delta.Z)<35+Ext.Z;
        }
        if (Hit)
        {
            if (O->Kind==EFDKind::Vine)
            {
                // The furnace must be restored before its suspended root seal can burn.
                if (O->ObjectId!=TEXT("r3_vine") || bFurnaceSolved) O->Ignite();
            }
            else if (O->RequiredView==INDEX_NONE) O->LitRemaining=.12f;
            else if (Flat && Player->FoldView==O->RequiredView && !O->bActivated)
            {
                O->bActivated=true;
                Notify(RoomSolved() ? TEXT("四向印记已全部点亮！展开为三维，在拱门前按 E 修复核心。") : FString::Printf(TEXT("%s印记已点亮。站稳后展开为三维，用 Q / F 寻找下一段阶梯。"),FDProjection::Name(O->RequiredView)),6);
                FDVisuals::Sound(this,TEXT("S_Restore"),.35f);
            }
        }
    }
}

bool AFDWorld::RoomSolved() const
{
    if (CurrentRoom==0) return bUsed2D;
    if (CurrentRoom==1) return bUsed2D && Find(TEXT("r1_pillar"))->bMoved;
    if (CurrentRoom==2) return Find(TEXT("r2_vine"))->bBurned;
    if (CurrentRoom==3) return bFurnaceSolved && Find(TEXT("r3_vine"))->bBurned;
    for (int32 I=0;I<4;++I) if (!Find(FString::Printf(TEXT("r4_seal%d"),I))->bActivated) return false;
    return Find(TEXT("r4_pillar"))->bMoved;
}

void AFDWorld::UseNearby()
{
    if (Player->b2D) { Notify(TEXT("请先展开为三维，再移动石柱或激活出口。")); return; }
    for (AFDObject* O : Objects)
    {
        if (O->Room!=CurrentRoom || O->Kind!=EFDKind::Pillar || O->bMoved) continue;
        if (FVector::Dist2D(Player->GetActorLocation(),O->GetActorLocation())<285)
        { O->Slide(); return; }
    }
    AFDObject* Exit=Find(FString::Printf(TEXT("exit%d"),CurrentRoom));
    if (Exit && FVector::Dist(Player->GetActorLocation(),Exit->GetActorLocation())<220)
    {
        if (!RoomSolved()) { Notify(Objective(),4); return; }
        FDVisuals::Sound(this,TEXT("S_Restore"),.55f);
        if (CurrentRoom<4)
        { EnterRoom(CurrentRoom+1,true); Notify(TEXT("本关已修复，通关进度已保存。"),4); }
        else
        {
            bFinished=true; Save->Room=4; Save->Finished=true; Persist();
            Player->GetCharacterMovement()->StopMovementImmediately();
            Player->GetCharacterMovement()->SetComponentTickEnabled(false);
        }
        return;
    }
    Notify(TEXT("靠近带标记的石柱或出口拱门，再按 E 交互。"));
}

FString AFDWorld::ContextAction() const
{
    if (!Player || Player->b2D) return FString();
    for (AFDObject* O : Objects)
        if (O->Room==CurrentRoom && O->Kind==EFDKind::Pillar && !O->bMoved && FVector::Dist2D(Player->GetActorLocation(),O->GetActorLocation())<285)
            return TEXT("E  /  移动石柱");
    AFDObject* Exit=Find(FString::Printf(TEXT("exit%d"),CurrentRoom));
    if (Exit && FVector::Dist(Player->GetActorLocation(),Exit->GetActorLocation())<220)
        return RoomSolved() ? TEXT("E  /  修复并前进") : TEXT("出口仍被封印");
    return FString();
}

void AFDWorld::Persist()
{
    if (!bSmoke && !bCapture && !UGameplayStatics::SaveGameToSlot(Save,TEXT("FoldlightTrial_v1"),0))
        Notify(TEXT("进度保存失败，本次游戏仍可继续。"));
}
void AFDWorld::Notify(const FString& Message,float Duration) { Toast=Message; ToastRemaining=Duration; }
FString AFDWorld::RoomTitle() const
{
    static const TCHAR* Names[]={TEXT("初次折叠"),TEXT("错位之桥"),TEXT("余烬花园"),TEXT("双生熔炉"),TEXT("四向折光塔")};
    return Names[FMath::Clamp(CurrentRoom,0,4)];
}
FString AFDWorld::Objective() const
{
    switch(CurrentRoom)
    {
    case 0: return TEXT("折叠错位的平台，在远端拱门前展开为三维。");
    case 1: return Find(TEXT("r1_pillar"))->bMoved ? TEXT("石柱已就位，折叠出一条跨越断层的路。") : TEXT("在三维中找到轨道石柱，将它移到阶梯缺口。");
    case 2: return Find(TEXT("r2_vine"))->bBurned ? TEXT("藤蔓封印已解除，前往出口拱门。") : TEXT("靠近藤蔓，按 G 施放已觉醒的火焰印记。");
    case 3: return !bFurnaceSolved ? TEXT("用一枚三维火印，同时点燃两座低位熔炉。") : (Find(TEXT("r3_vine"))->bBurned ? TEXT("两道封印均已解除，折叠平台前往出口。") : TEXT("将火印折叠为高墙，烧毁悬空的藤蔓。"));
    default:
        if (!Find(TEXT("r4_pillar"))->bMoved) return TEXT("先移动轨道石柱，再以南视折叠，开始换面登塔。");
        for (int32 View : {0,3,2,1})
            if (!Find(FString::Printf(TEXT("r4_seal%d"),View))->bActivated)
                return FString::Printf(TEXT("前往%s祭台，以同向二维火墙点亮印记；按 H 查看路线。"),FDProjection::Name(View));
        return TEXT("四向印记已点亮！在最高祭台展开为三维，按 E 修复核心。");
    }
}
void AFDWorld::RequestHint()
{
    HintLevel=FMath::Min(HintLevel+1,3);
    static const TCHAR* Hints[5][3]={
        {TEXT("金边平台的轮廓，可以在同一个平面上相连。"),TEXT("Q / F 可选择四个观察方向；南北压缩前后，东西压缩左右。"),TEXT("选择南视后按住再松开 V，向右跳跃；到终点展开为三维，在拱门前按 E。")},
        {TEXT("带标记的石柱，连接着一条金色轨道。"),TEXT("保持三维，靠近起始平台左后方的石柱。"),TEXT("靠近石柱按 E，等它停稳后按住再松开 V，沿对齐的阶梯跳跃。")},
        {TEXT("藤蔓已经干枯，留意琥珀色的施法范围预览。"),TEXT("火印会出现在蜡烛前方一小段距离处。"),TEXT("面朝藤蔓，在约两米外按 G；烧毁藤蔓后，前往远端拱门按 E。")},
        {TEXT("两座低位熔炉，需要同时受到火焰加热。"),TEXT("宽阔的三维火印能覆盖两座熔炉，狭窄的二维火墙则不行。"),TEXT("在三维中将火印放到双炉之间；解封后站到高处藤蔓左侧，切换二维再按 G。")},
        {TEXT("登塔路线依次是南视、西视、北视、东视；每到祭台，朝屏幕右侧施放火墙。"),TEXT("在祭台展开三维，Q / F 换方向，再折叠；新的阶梯会出现在屏幕右侧。"),TEXT("先按 E 移动石柱。沿南→西→北→东登塔，各祭台按 G 点亮印记，最后展开并按 E。")}
    };
    Notify(Hints[CurrentRoom][HintLevel-1],9);
}

void AFDWorld::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!Player) Player=Cast<AFDCharacter>(UGameplayStatics::GetPlayerPawn(this,0));
    if (!Player) return;
    Player->Director=this;
    if (!bInitialized)
    {
        bInitialized=true;
        Player->SetActorLocation(RoomOrigin(CurrentRoom)+FVector(-1100,0,90));
        Player->GetController()->SetControlRotation(FRotator(-27,-90,0));
        if (bSmoke || bCapture) Start(true);
    }
    if (bSmoke || bCapture) RunSmoke(DeltaSeconds);
    if (!bStarted || bPaused || bFinished) { PreviewRoot->SetVisibility(false,true); return; }
    SessionTime+=DeltaSeconds;
    CastCooldown=FMath::Max(0.f,CastCooldown-DeltaSeconds);
    ToastRemaining=FMath::Max(0.f,ToastRemaining-DeltaSeconds);
    UpdateFire(DeltaSeconds);
    if (CurrentRoom==3 && !bFurnaceSolved)
    {
        if (Find(TEXT("r3_left"))->LitRemaining>0 && Find(TEXT("r3_right"))->LitRemaining>0) FurnaceHold+=DeltaSeconds;
        else FurnaceHold=0;
        if (FurnaceHold>.65f) { bFurnaceSolved=true; Notify(TEXT("双生熔炉已修复，接下来烧毁高处的藤蔓封印。"),5); }
    }
    if (Player->GetActorLocation().Z<-600) { ResetCurrentRoom(true); return; }
    const bool ShowPreview=CurrentRoom>=2 && FireRemaining<=0 && !Player->bPreview;
    PreviewRoot->SetVisibility(ShowPreview,true);
    if (ShowPreview)
    {
        PreviewRoot->SetWorldLocation(GetCastAnchor());
        const bool Flat=Player->b2D;
        PreviewRoot->SetWorldRotation(Flat ? FDProjection::Right(Player->FoldView).Rotation() : FRotator::ZeroRotator);
        const float Width=Flat ? 120 : 400;
        for (int I=0;I<4;++I)
        {
            FVector P,S;
            if (Flat)
            {
                P=I<2 ? FVector((I ? 1 : -1)*Width*.5f,0,220) : FVector(0,0,I==2 ? 0 : 440);
                S=I<2 ? FVector(3,3,440) : FVector(Width,3,3);
            }
            else
            {
                P=I<2 ? FVector((I ? 1 : -1)*200,0,4) : FVector(0,(I==2 ? -1 : 1)*200,4);
                S=I<2 ? FVector(3,400,3) : FVector(400,3,3);
            }
            PreviewPieces[I]->SetRelativeLocation(P); PreviewPieces[I]->SetRelativeScale3D(S/100);
        }
    }
}

AFDGameMode::AFDGameMode()
{
    DefaultPawnClass=AFDCharacter::StaticClass(); HUDClass=AFDHUD::StaticClass();
}
void AFDGameMode::StartPlay()
{
    Super::StartPlay();
    if (!UGameplayStatics::GetActorOfClass(this,AFDWorld::StaticClass())) GetWorld()->SpawnActor<AFDWorld>();
}
