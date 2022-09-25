# UE5_Lec_FifthProject_FirstPersonShooter

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
 ### QUESTION
