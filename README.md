# UE5_Lec_FifthProject_FirstPersonShooter

Readmeに託けた自分用のメモです
### getter
 PrivateでUSpringArmとかを作って、PublicにGetUSAなどを作るとアクセスしやすい。

###  GetUnitAxisの引数
x軸から数えた回転なのかy軸から数えた回転なのか。これによって前向きに動くか、横向きに動くかを決めている。

### mouse input deltatime
いらない

### Controller Rotation
controllerはデフォルトでRootComponentを回すように設計されてるから、bUseControllerRotationをfalseにすればRootが回らないようになる。

### socket
USkeletalMeshSocketがコントロールをしてる

### Quat
```const FQuat Rotation {SocketTransform.GetRotation()};
const FVector RotationAxis {Rotation.GetAxisX()};
 ```
これはRotationを得て、元々のxだった方向を求めてる


### DeprojectScreenToWorld
``` UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);```
これで画面から世界に投影してる。
### Super::NativeInitializeAnimation();
これがaniminstansのコンストラクター

### WorldTimerManager
これが何をしているのか調べる必要あり。

 ### QUESTION
