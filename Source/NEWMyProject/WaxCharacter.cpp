#include "WaxCharacter.h"
#include "DimensionManagerComponent.h"
#include "Magic_FireRing.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

AWaxCharacter::AWaxCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // 角色不随控制器旋转，仅受移动方向影响
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;

    // 自动朝向移动方向（2D模式下实现自动转身）
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); 

    // 赋予角色在空中的转向与移动控制权 (1.0代表完全控制，0.0代表无法控制)
    GetCharacterMovement()->AirControl = 0.8f;

    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
    SpringArmComponent->SetupAttachment(RootComponent);
    SpringArmComponent->TargetArmLength = 400.0f; 
    SpringArmComponent->bUsePawnControlRotation = true; 

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
    CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
    CameraComponent->bUsePawnControlRotation = false; 

    // 挂载维度管理器
    DimensionManager = CreateDefaultSubobject<UDimensionManagerComponent>(TEXT("DimensionManager"));
}

void AWaxCharacter::BeginPlay()
{
    Super::BeginPlay();

    // 挂载增强输入映射上下文
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
        {
            if (DefaultMappingContext)
            {
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }
}

void AWaxCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWaxCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (MoveAction)
        {
            EnhancedInputComp->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AWaxCharacter::Move);
        }

        if (LookAction)
        {
            EnhancedInputComp->BindAction(LookAction, ETriggerEvent::Triggered, this, &AWaxCharacter::Look);
        }

        if (JumpAction)
        {
            EnhancedInputComp->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
            EnhancedInputComp->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
        }

        if (SwitchDimensionAction)
        {
            EnhancedInputComp->BindAction(SwitchDimensionAction, ETriggerEvent::Started, this, &AWaxCharacter::SwitchDimension);
        }

        if (CastMagicAction)
        {
            EnhancedInputComp->BindAction(CastMagicAction, ETriggerEvent::Started, this, &AWaxCharacter::CastMagic);
        }
    }
}

void AWaxCharacter::Look(const FInputActionValue& Value)
{
    // 只有在 3D 模式下，才允许鼠标自由控制视角
    if (DimensionManager && DimensionManager->GetCurrentDimension() == EDimensionState::Dimension3D)
    {
        FVector2D LookAxisVector = Value.Get<FVector2D>();

        if (Controller != nullptr)
        {
            AddControllerYawInput(LookAxisVector.X);
            AddControllerPitchInput(LookAxisVector.Y);
        }
    }
}

void AWaxCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr && DimensionManager != nullptr)
    {
        EDimensionState CurrentState = DimensionManager->GetCurrentDimension();

        if (CurrentState == EDimensionState::Dimension3D)
        {
            // --- 3D 自由移动 ---
            const FRotator Rotation = Controller->GetControlRotation();
            const FRotator YawRotation(0, Rotation.Yaw, 0);

            const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
            const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

            AddMovementInput(ForwardDirection, MovementVector.Y);
            AddMovementInput(RightDirection, MovementVector.X);
        }
        else if (CurrentState == EDimensionState::Dimension2D)
        {
            // --- 2.5D 横版移动 ---
            // 物理移动已被 PlaneConstraint 锁定，我们仅应用相对于相机的左右推力。
            // 获取进入2D瞬间动态锁定的相机方向
            FRotator CamRot = DimensionManager->GetSaved2DYawRotation();
            // 相机的正右方就是我们的横版移动轴
            const FVector RightDirection = FRotationMatrix(CamRot).GetUnitAxis(EAxis::Y);
            
            // 按 A 时 X 值为负，向相机的左侧移动；按 D 时 X 值为正，向相机的右侧移动。
            AddMovementInput(RightDirection, MovementVector.X);
        }
    }
}

void AWaxCharacter::SwitchDimension(const FInputActionValue& Value)
{
    if (DimensionManager)
    {
        DimensionManager->ToggleDimension();
    }
}

void AWaxCharacter::CastMagic(const FInputActionValue& Value)
{
    UE_LOG(LogTemp, Warning, TEXT("[WaxCharacter] CastMagic() called! G key is working."));

    if (!MagicFireRingClass)
    {
        UE_LOG(LogTemp, Error, TEXT("[WaxCharacter] MagicFireRingClass is NULL! Please set it in the blueprint Details panel."));
        return;
    }
    if (!GetWorld()) return;

    // 在角色前方 300cm 处生成火环魔法
    FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 300.0f;
    FRotator SpawnRotation = GetActorRotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AMagic_FireRing* FireRing = GetWorld()->SpawnActor<AMagic_FireRing>(
        MagicFireRingClass,
        SpawnLocation,
        SpawnRotation,
        SpawnParams
    );

    if (FireRing && DimensionManager)
    {
        // 自动检测当前维度，传入对应模式
        bool bIs2D = (DimensionManager->GetCurrentDimension() == EDimensionState::Dimension2D);
        FireRing->InitializeMagic(bIs2D);
    }
}
