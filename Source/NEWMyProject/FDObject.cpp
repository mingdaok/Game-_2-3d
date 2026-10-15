#include "FoldlightDemo.h"
#include "FDProjection.h"
#include "FDVisuals.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

void FDVisuals::Sound(AActor* Owner,const TCHAR* Name,float Volume)
{
    USoundBase* Audio=LoadObject<USoundBase>(nullptr,*FString::Printf(TEXT("/Game/Foldlight/Audio/%s.%s"),Name,Name));
    if (Audio) UGameplayStatics::PlaySound2D(Owner,Audio,Volume);
}

UMaterialInstanceDynamic* FDVisuals::Material(UObject* Owner, FLinearColor Color, float Glow)
{
    UMaterialInterface* Parent = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Foldlight/Art/M_Surface.M_Surface"));
    if (!Parent) Parent = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    UMaterialInstanceDynamic* Result = UMaterialInstanceDynamic::Create(Parent, Owner);
    if (Result)
    {
        Result->SetVectorParameterValue(TEXT("Tint"), Color);
        Result->SetVectorParameterValue(TEXT("Color"), Color);
        Result->SetVectorParameterValue(TEXT("Glow"), Color * (Glow + .32f));
        const bool Brass=Color.R>Color.B*2 && Color.G>Color.B*1.5f;
        Result->SetScalarParameterValue(TEXT("Metallic"),Brass ? .55f : 0.f);
        Result->SetScalarParameterValue(TEXT("Roughness"),Brass ? .36f : .82f);
    }
    return Result;
}

UStaticMeshComponent* FDVisuals::Piece(AActor* Owner, USceneComponent* Parent, const TCHAR* Shape, FVector Position, FVector Size, FLinearColor Color, float Glow)
{
    UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(Owner);
    Owner->AddInstanceComponent(C);
    C->SetupAttachment(Parent);
    C->SetMobility(EComponentMobility::Movable);
    C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    const bool Authored=FString(Shape).StartsWith(TEXT("SM_"));
    const FString MeshPath=Authored ? FString::Printf(TEXT("/Game/Foldlight/Art/Models/%s.%s"),Shape,Shape) : FString::Printf(TEXT("/Engine/BasicShapes/%s.%s"),Shape,Shape);
    C->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,*MeshPath));
    UMaterialInstanceDynamic* Mat=Material(Owner, Color, Glow);
    if (FString(Shape)==TEXT("SM_Stone") || FString(Shape)==TEXT("SM_Arch") || FString(Shape)==TEXT("SM_Pedestal")) Mat->SetScalarParameterValue(TEXT("Grain"),1.f);
    if (FString(Shape)==TEXT("SM_Root") || FString(Shape)==TEXT("SM_Leaf") || FString(Shape)==TEXT("SM_Grass"))
    { Mat->SetScalarParameterValue(TEXT("Metallic"),0); Mat->SetScalarParameterValue(TEXT("Roughness"),.95f); }
    C->SetMaterial(0, Mat);
    C->SetRelativeLocation(Position);
    C->SetRelativeScale3D(Size / 100.0f);
    C->RegisterComponent();
    return C;
}

AFDObject::AFDObject()
{
    PrimaryActorTick.bCanEverTick = true;
    Solid = CreateDefaultSubobject<UBoxComponent>(TEXT("SourceCollision"));
    SetRootComponent(Solid);
    Solid->SetCollisionProfileName(TEXT("BlockAll"));
    VisualRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Visual"));
    VisualRoot->SetupAttachment(Solid);
    Projection = CreateDefaultSubobject<UBoxComponent>(TEXT("ProjectedCollision"));
    Projection->SetupAttachment(Solid);
    Projection->SetCollisionProfileName(TEXT("BlockAll"));
    Projection->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Projection->SetHiddenInGame(true);
}

bool AFDObject::IsWalkable() const { return Kind == EFDKind::Platform || Kind == EFDKind::Pillar; }
bool AFDObject::Blocks() const { return IsWalkable() || (Kind == EFDKind::Vine && !bBurned); }

void AFDObject::Configure(AFDWorld* World, EFDKind Type, int32 RoomId, const FString& Id, FVector Location, FVector Dimensions, FLinearColor Color)
{
    Director = World; Kind = Type; Room = RoomId; ObjectId = Id; Size = Dimensions;
    InitialLocation = Location;
    SetActorLocation(Location);
    Solid->SetBoxExtent(Size * .5f);
    Solid->SetCollisionEnabled(Blocks() ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
    const auto Piece = [&](const TCHAR* Shape, FVector Pos, FVector S, FLinearColor C, float Glow = 0.f)
    {
        UStaticMeshComponent* M = FDVisuals::Piece(this, VisualRoot, Shape, Pos, S, C, Glow);
        Pieces.Add(M); OriginalMaterials.Add(Cast<UMaterialInstanceDynamic>(M->GetMaterial(0))); return M;
    };
    if (IsWalkable())
    {
        UStaticMeshComponent* Main = Piece(TEXT("SM_Stone"), FVector::ZeroVector, Size, Color);
        Surface = Cast<UMaterialInstanceDynamic>(Main->GetMaterial(0));
        const float Top=Size.Z*.5f;
        const FLinearColor Decks[]={FLinearColor(.36f,.49f,.45f),FLinearColor(.36f,.42f,.46f),FLinearColor(.31f,.44f,.34f),FLinearColor(.43f,.35f,.28f),FLinearColor(.34f,.40f,.49f)};
        Piece(TEXT("SM_Stone"),FVector(0,0,Top-9),FVector(Size.X,Size.Y,18),Decks[FMath::Clamp(Room,0,4)]);
        Piece(TEXT("SM_Stone"),FVector(0,0,-Top+10),FVector(Size.X*.96f,Size.Y*.96f,20),Color*.7f);
        // Four matching fascias keep the playable lip legible from every view.
        for (int Side=0;Side<4;++Side)
        {
            const FVector N=FDProjection::Depth(Side), R=FDProjection::Right(Side);
            const float Span=FDProjection::Width(Size,Side), Depth=FVector::DotProduct(Size,N.GetAbs());
            UStaticMeshComponent* Band=Piece(TEXT("Cube"),N*(Depth*.5f+.4f)+FVector(0,0,Top-23),FVector(Span*.92f,2,3),FDVisuals::Gold,.35f);
            Band->SetRelativeRotation(R.Rotation());
            const int Count=FMath::Clamp(FMath::RoundToInt(Span/160),2,12);
            for (int J=0;J<Count;++J)
            {
                const FVector P=N*(Depth*.5f+.5f)+R*((J+.5f)*Span/Count-Span*.5f)+FVector(0,0,Top-40);
                Piece(TEXT("SM_Stone"),P,FVector(18,18,17),Color*.7f)->SetRelativeRotation(FRotator(0,45,0));
            }
        }
        // Shallow dark joints subdivide wide platforms without changing collision.
        const int Tiles=FMath::Clamp(FMath::CeilToInt(Size.X/220),2,14);
        for (int J=1;J<Tiles;++J)
            Piece(TEXT("Cube"),FVector(-Size.X*.5f+J*Size.X/Tiles,0,Top+.2f),FVector(2,Size.Y*.90f,.4f),Color*.42f);
        for (float Y : {-.3f,0.f,.3f})
            Piece(TEXT("Cube"),FVector(0,Size.Y*Y,Top+.25f),FVector(Size.X*.92f,2,.5f),Color*.42f);
        Piece(TEXT("SM_Ring"),FVector(0,0,Top+.7f),FVector(90,90,1),FDVisuals::Gold,.12f);
        FRandomStream DetailSeed(FCrc::StrCrc32(*Id));
        // Low, collision-free weathering stays at the corners, clear of jumps.
        for (int I=0;I<4;++I)
        {
            const FVector P(Size.X*(I%2 ? .40f : -.40f),Size.Y*(I<2 ? .39f : -.39f),Top+.4f);
            if (Room!=3)
            {
                Piece(TEXT("SM_Grass"),P,FVector(48,48,Room==2 ? 25 : 13),FLinearColor(.13f,.24f,.14f));
                Piece(TEXT("SM_Leaf"),P+FVector(10,0,1),FVector(27,35,36),FLinearColor(.17f,.28f,.15f))->SetRelativeRotation(FRotator(70,DetailSeed.FRandRange(0,360),0));
            }
            for (int J=0;J<2;++J)
                Piece(TEXT("SM_Stone"),P+FVector(DetailSeed.FRandRange(-28,28),DetailSeed.FRandRange(-22,22),1),FVector(DetailSeed.FRandRange(10,24),12,3),Color*.55f)->SetRelativeRotation(FRotator(0,DetailSeed.FRandRange(0,180),0));
            Piece(TEXT("Cube"),P+FVector(-20,15,0),FVector(45,1.2f,.5f),Color*.35f)->SetRelativeRotation(FRotator(0,35+I*27,0));
        }
        if (Kind == EFDKind::Pillar)
        {
            Piece(TEXT("Cube"), FVector(0, Size.Y*.5f+4, 0), FVector(65, 8, 65), FDVisuals::Gold, .3f)->SetRelativeRotation(FRotator(0,0,45));
        }
    }
    else if (Kind == EFDKind::Vine)
    {
        for (int I=0; I<7; ++I)
        {
            float Y = (I-3) * Size.Y / 7;
            UStaticMeshComponent* Branch = Piece(TEXT("SM_Root"), FVector(FMath::Sin(I*2.f)*12,Y,0), FVector(105,105,Size.Z), FLinearColor(.24f,.10f,.055f));
            Branch->SetRelativeRotation(FRotator(0,0,(I%2 ? 13 : -13)));
            for (int K=0;K<3;++K)
                Piece(TEXT("SM_Crystal"),FVector(18,Y+12,Size.Z*(K*.23f-.25f)),FVector(28,45,25),FLinearColor(.39f,.20f,.08f),.15f)->SetRelativeRotation(FRotator(0,0,45+K*35));
        }
        Piece(TEXT("Cube"), FVector(0,0,-Size.Z*.3f), FVector(Size.X,Size.Y,18), FLinearColor(.19f,.075f,.04f));
        Piece(TEXT("Cube"), FVector(0,0,Size.Z*.2f), FVector(Size.X,Size.Y,18), FLinearColor(.19f,.075f,.04f));
    }
    else if (Kind == EFDKind::Receiver)
    {
        Piece(TEXT("SM_Pedestal"), FVector(0,0,-18), FVector(100,100,38), FDVisuals::Stone);
        Piece(TEXT("SM_Ring"),FVector(0,0,7),FVector(75,75,6),FDVisuals::Gold,.2f);
        for (int I=0;I<4;++I)
        {
            const float A=I*PI*.5f;
            Piece(TEXT("SM_Crystal"),FVector(FMath::Cos(A)*33,FMath::Sin(A)*33,20),FVector(14,14,48),FDVisuals::Gold,.1f);
        }
        UStaticMeshComponent* Core = Piece(TEXT("SM_Crystal"), FVector(0,0,30), FVector(43,43,65), FDVisuals::Gold, .15f);
        Surface = Cast<UMaterialInstanceDynamic>(Core->GetMaterial(0));
    }
    else if (Kind == EFDKind::Exit)
    {
        Piece(TEXT("SM_Pedestal"), FVector(0,0,-8), FVector(205,205,26), FDVisuals::Stone);
        for (int I=0;I<12;++I)
        {
            const float A = I * PI / 6;
            Piece(TEXT("Cube"), FVector(FMath::Cos(A)*88,FMath::Sin(A)*88,8), FVector(20,8,6), FDVisuals::Gold, 1.5f)->SetRelativeRotation(FRotator(0,FMath::RadiansToDegrees(A),0));
        }
        for (float X : {-104.f,104.f})
        {
            Piece(TEXT("SM_Pedestal"),FVector(X,0,75),FVector(42,42,150),FDVisuals::Stone);
            Piece(TEXT("SM_Crystal"),FVector(X,0,164),FVector(25,25,50),FDVisuals::Edge,1.2f);
        }
        Piece(TEXT("SM_Ring"),FVector(0,-10,165),FVector(280,280,13),FDVisuals::Gold,.25f)->SetRelativeRotation(FRotator(0,0,90));
        Piece(TEXT("SM_Ring"),FVector(0,-10,165),FVector(241,241,4),FDVisuals::Edge,1.1f)->SetRelativeRotation(FRotator(0,0,90));
        for (int I=0;I<12;++I)
        {
            const float A=I*PI/6;
            Piece(TEXT("SM_Stone"),FVector(FMath::Cos(A)*131,-20,165+FMath::Sin(A)*131),FVector(10,8,18),FDVisuals::Gold,.6f)->SetRelativeRotation(FRotator(0,0,FMath::RadiansToDegrees(A)-90));
        }
        UStaticMeshComponent* Core = Piece(TEXT("SM_Crystal"), FVector(0,-8,264), FVector(24,24,42), FDVisuals::Gold, 3);
        Surface = Cast<UMaterialInstanceDynamic>(Core->GetMaterial(0));
    }
}

void AFDObject::SetProjection(bool Enabled, float PlaneDepth, int32 View)
{
    bProjected = Enabled;
    Solid->SetCollisionResponseToChannel(ECC_Pawn, Enabled ? ECR_Ignore : ECR_Block);
    Solid->SetCollisionEnabled(Blocks() ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
    Projection->SetBoxExtent(FDProjection::Right(View).GetAbs()*FDProjection::Width(Size,View)*.5f+FDProjection::Depth(View).GetAbs()*70+FVector(0,0,Size.Z*.5f));
    Projection->SetWorldLocation(FDProjection::OnPlane(GetActorLocation(),View,PlaneDepth));
    Projection->SetCollisionEnabled(Enabled && Blocks() ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
}

void AFDObject::Ignite()
{
    if (Kind != EFDKind::Vine || bBurning || bBurned) return;
    bBurning = true; BurnTime = 1.5f;
    for (UStaticMeshComponent* M : Pieces) M->SetMaterial(0, FDVisuals::Material(this, FDVisuals::Gold, 2));
    if (Director) Director->Notify(TEXT("藤蔓正在燃烧，它的投影也会一同消失。"));
    if (UNiagaraSystem* FX = LoadObject<UNiagaraSystem>(nullptr,TEXT("/Game/Vefects/Free_Fire/Shared/Particles/NS_Fire_Big_Simple.NS_Fire_Big_Simple")))
    {
        UNiagaraComponent* C = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), FX, GetActorLocation()-FVector(0,0,Size.Z*.5f), FRotator::ZeroRotator, FVector(.7f), true);
        if (C) C->SetFloatParameter(TEXT("User.Lifetime"), 1.5f);
        // Spawned effects are owned separately and explicitly removed at burn completion below.
        if (C) C->AttachToComponent(VisualRoot, FAttachmentTransformRules::KeepWorldTransform);
    }
}

void AFDObject::Slide()
{
    if (Kind != EFDKind::Pillar || bSliding || bMoved) return;
    bSliding = true;
    if (Director) Director->Notify(TEXT("石柱正在沿轨道移动，留意侧面的平台轮廓。"));
}

void AFDObject::ResetObject()
{
    bMoved = bSliding = bBurning = bBurned = bLit = bActivated = false;
    BurnTime = LitRemaining = 0;
    SetActorLocation(InitialLocation);
    VisualRoot->SetVisibility(true,true);
    TArray<USceneComponent*> AttachedPieces;
    VisualRoot->GetChildrenComponents(true, AttachedPieces);
    for (USceneComponent* C : AttachedPieces) if (UNiagaraComponent* FX = Cast<UNiagaraComponent>(C)) FX->DestroyComponent();
    if (Kind == EFDKind::Vine)
        for (int32 I=0;I<Pieces.Num();++I) Pieces[I]->SetMaterial(0,OriginalMaterials[I]);
    SetProjection(false,0);
}

void AFDObject::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!Director || Director->bPaused || !Director->bStarted || Director->CurrentRoom != Room) return;
    if (bSliding)
    {
        SetActorLocation(FMath::VInterpConstantTo(GetActorLocation(),SlideTarget,DeltaSeconds,360));
        if (GetActorLocation().Equals(SlideTarget,1)) { SetActorLocation(SlideTarget); bSliding=false; bMoved=true; }
    }
    if (bBurning)
    {
        BurnTime -= DeltaSeconds;
        if (BurnTime<=0)
        {
            bBurned=true; bBurning=false;
            VisualRoot->SetVisibility(false,true);
            TArray<USceneComponent*> AttachedPieces; VisualRoot->GetChildrenComponents(true,AttachedPieces);
            for (USceneComponent* C : AttachedPieces) if (UNiagaraComponent* FX=Cast<UNiagaraComponent>(C)) FX->DestroyComponent();
            SetProjection(bProjected,Director->Player ? Director->Player->PlaneDepth : 0,Director->Player ? Director->Player->FoldView : 0);
        }
    }
    if (Kind==EFDKind::Receiver)
    {
        LitRemaining=FMath::Max(0.f,LitRemaining-DeltaSeconds); bLit=bActivated || LitRemaining>0;
        if (Surface) Surface->SetVectorParameterValue(TEXT("Glow"),FDVisuals::Gold*(bLit ? 6.f : .15f));
    }
    if (Kind==EFDKind::Exit && Surface)
        Surface->SetVectorParameterValue(TEXT("Glow"),(Director->RoomSolved() ? FDVisuals::Edge : FDVisuals::Gold)*3.f);
}
