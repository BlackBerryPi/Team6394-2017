#include <IterativeRobot.h>
#include <Joystick.h>
#include <Solenoid.h>
#include <Talon.h>
#include <DoubleSolenoid.h>
#include <LiveWindow/LiveWindow.h>
#include <RobotDrive.h>
#include <Timer.h>
#include <math.h>

//��ť�趨
const int ButNum = 12;
bool LastButState[ButNum];//�����ϴΰ�ť
bool ThisButState[ButNum];//�����ϴΰ�ť
bool ButStateChange[ButNum];//��ť�仯

enum DefineBut {
	CollectBallBut = 2,//����Ħ���ְ�ť
	CallibrateBut = 6,//�����Զ�У׼��ť
	ShootBallBut = 4,//����ť
	CogSolenoidDownBut = 3,//�����ռ���ť
	PutCogBut = 5;//�ҳ��ְ�ť
FastModeBut = 10,//����ģʽ�л���ť
SlowModeBut = 9,//����ģʽ�л���ť
FastClimbBut = 8,//��������
SlowClimbBut = 7//��������
} ButSetting;

//�����������������趨

//CAN
enum DefineCANPin {
	BotCogSolenoidPin = 0//�ײ�����DIO0
	UpCogSolenoidPin = 1;//�ϲ�������DIO2
} CANSetting;

///DIO
enum DefineDIOPin {
	CogCollectedPin = 0,//��������΢������DIO
	CogDistancePin = 2;//�����൲��
} DIOSetting;

///PWM
enum DefinePWMPin {
	RBaseMotorPin = 0,//PWM0��������
	LBaseMotorPin = 1,//PWM1��������
	CogFriWheelPin = 2,//����Ħ����PWM2
	CollectBallWheelPin = 3,//����Ħ����PWM3
	SBTransferBeltPin = 4,//���ʹ�PWM4
	SBTransferWheelPin = 5,//����С��PWM5
	SBShootWheelPin = 6,//������PWM6
	SBAngleMotorPin = 7,//����Ƕȵ��PWM7
	ClimbingMotorPin = 8//�������PWM8
} PWMSetting;

//�����������������趨
const double CogFriWheelSpeed = 1;//����Ħ��������
const double SpeedLowLimit = 0.3;//�����ٶ�����
const double AngleCoe = 0.2;//�Ƕȵ���Pϵ��
const int AngleTolerate = 10;//�Ƕȵ������̷�Χ
const int AngleSliderCoe = 50;//�Ƕȵ�����Ƭ��������
const double SlowClimbSpeed = 0.3;//��������ת��
const double FastClimbSpeed = 1;//��������ת��
const double MeterConvert = 1.3;
const double AutoMovingSpeed = 0.8;//�Զ��ƶ��ٶ�

								   //������������״̬�趨
bool BotSolenoidDown = false;
bool UpSolenoidDown = false;
bool CogCollected = false;
bool FastMode = true;
int AnglePosition = 0;
bool Calibrated = false;


//�������������׶κ���
class Robot : public frc::IterativeRobot {
public:
	Robot() {
		myRobot.SetExpiration(0.1);
		timer.Start();
	}

private:
	frc::RobotDrive myRobot{ RightBaseMotorPin, LeftBaseMotorPin };  // Robot drive system
	frc::Joystick stick{ 0 };         // Only joystick
	frc::LiveWindow* lw = frc::LiveWindow::GetInstance();
	frc::Timer timer;

	frc::Solenoid BotCogSolenoid{ BotCogSolenoidPin };
	frc::Solenoid UpCogSolenoid{ UpCogSolenoidPin };

	frc::Talon CogFriWheel{ CogFriWheelPin };//����Ħ����
	frc::Talon BallFriWheel{ CollectBallWheelPin };//����Ħ����
	frc::Talon BallTransferBelt{ SBTransferBeltPin };//���ʹ�
	frc::Talon BallTransferWheel{ SBTransferWheelPin };//����С��
	frc::Talon ShootWheel{ SBShootWheelPin };//������
	frc::Talon AngleMotor{ SBAngleMotorPin };//�Ƕȵ������
	frc::Talon ClimbingMotor{ ClimbingMotorPin };//�������

	void AutonomousInit() override {
		timer.Reset();
		timer.Start();
	}

	void AutonomousPeriodic() override {
		// Drive for 2 seconds
		if (timer.Get() < 2.0) {
			myRobot.Drive(-0.5, 0.0);  // Drive forwards half speed
		}
		else {
			myRobot.Drive(0.0, 0.0);  // Stop robot
		}
	}

	void TeleopInit() override {
		int i = 1;
		for{i = 1; i<ButNum; i++} {
			LastButState[i] = false;
		}
	}

	void TeleopPeriodic() override {
		// Drive with arcade style (use right stick)

		SensorUpdate();

		//����
		myRobot.ArcadeDrive(stick);

		///�ճ���
		if (NewCommend(CogSolenoidDownBut)) {
			if (BotSolenoidDown) {
				LeftSolenoid();//�Ѿ������������ѡ����̧
			}
			else {
				PutDownSolenoid()��//δ�����������ѡ���½�
			}
		}

		///�ҳ���
		if (ThisButState[PutCogBut]) {
			if (NewCommend(PutCogBut) && BotSolenoidDown) {
				LeftSolenoid();//�Ѿ������������ѡ����̧
			}
			else {
				PutCog()��//�ҳ���
			}
		}


		if (CogCollected&&SolenoidDown)LeftSolenoid();//���ֽ�����Զ�̧��
		if (ThisButState[FastModeBut])FastMode = true;//�л�Ϊ����ģʽ
		if (ThisButState[SlowModeBut])FastMode = false;//�л�Ϊ΢��ģʽ
		if (ThisButState[CallibrateBut])AngleMotor.Set(AngleCallibrate());//��ס�����Ƕ�
		if (ThisButState[ShootBallBut])ShootBall();//���
		if (ThisButState[FastClimbBut])CimbingMotor.Set(FastClimbSpeed);//��������
		if (ThisButState[SlowClimbBut])CimbingMotor.Set(SlowClimbSpeed);//��������

	}
	bool NewCommend(int ButIndex) {
		//���true�����һ���µ�true�ź�
		return ((ThisButState[ButIndex]) && (!LastButState[ButIndex]));
	}
	void TestPeriodic() override {
		lw->Run();
	}
	//���������������º���
	///�ܸ���
	void SensorUpdate() {
		//�˴����´�����״̬!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//CogCollected=
		//SolenoidDown=
		MapUpdate();
		ReadFromRaspberryPi();
		for{i = 1; i<ButNum; i++} {
			ThisButState[i] = stick.GetRawButton(i);
			ButStateChange[i] = (ThisButState[i] != LastButState[i])
		}
	}

	///��ͼ
	const int x = 0;
	const int y = 1;

	double Pos[] = { 0,0 };
	double ThisFacingAngle = 0;

	double CogPos[] = { 100,200 };
	double CogAngle = 0;
	double BallPos[] = { 300,200 };

	void MapUpdate() {
		double ThisAcc[] = { 0,0 };
		double ThisVel[] = { 0,0 };
		double ThisTime = timer.Get();
		TimeChange = ThisTime - LastTime;

		/*
		��������������������������������������������������������������������������������
		�˴���ThisAcc,ThisFacingAngle����
		��ȡ����֮����л��� ʹ��MeterConvert����
		��
		ThisAcc/=MeterConvert;
		δ��ȡ�����Ļ���Ĭ��0��0
		*/
		ThisVel[x] = LastVel[x] + TimeChange*(LastAcc[x] + ThisAcc[x]) / 2;
		Pos[x] += TimeChange*(ThisVel[x] + LastVel[x]) / 2;
		ThisVel[y] = LastVel[y] + TimeChange*(LastAcc[y] + ThisAcc[y]) / 2;
		Pos[y] += TimeChange*(ThisVel[y] + LastVel[y]) / 2;

		LastAcc[x] = ThisAcc[x];
		LastAcc[y] = ThisAcc[y];
		LastVel[x] = ThisVel[x];
		LastVel[y] = ThisVel[y];
		LastTime = ThisTime;
	}


	//��������������������

	///��������
	bool ReadDIO(int pin) {
		///����true����false
		return;
	}

	///������
	void PutDownSolenoid() {
		//�ռ����� ������������ ת��Ħ����
		BotCogSolenoid.Set(true);
		UpCogSolenoid.Set(true);
		CogFriWheel.Set(CogFriWheelSpeed);
		UpSolenoidDown = true;
		BotSolenoidDown = true;
	}

	//�ҳ���

	void PutCog() {
		//�ҳ��� �������ײ����� �ر�Ħ����
		double MarchingSpeed = 0.4;
		if (ReadDIO(CogDistancePin) && (!BotSolenoidDown))
		{
			BotCogSolenoid.Set(true);
			UpCogSolenoid.Set(true);
			CogFriWheel.Set(0);
			BotSolenoidDown = true;
		}
		else {
			if (MoveToPos(CogPos[x], CogPos[y], CogAngle)) {
				myRobot.Drive(MarchingSpeed, 0);
			}
			else {
				CalculatePos();
				MoveToPos(CogPos[x], CogPos[y], CogAngle, AutoMovingSpeed);
			}
		}

	}
	void CalculatePos() {
		//���ֹҹ�����
		double direct_dis;
		double image_dis;
		double Standard_dis = 0.01;
		double Hook_dis = 0.5;

		ReadFromRaspberryPi();
		//��ȡimage_dis;
		image_dis = image_dis*direct_dis*Standard_dis;
		CogPos[x] = Pox[x] + direct_dis*cos(ThisFacingAngle) + image_dis*sin(ThisFacingAngle);
		CogPos[y] = Pox[y] + direct_dis*sin(ThisFacingAngle) + image_dis*cos(ThisFacingAngle);
	}
	bool MoveToPos(double XPos, double YPos, double FinalFacingAngle, double MovingSpeed) {
		double TargetAngle;
		double x_dif;
		double y_dif;
		double angletol = 0.04;
		double angleact = 0.5;
		double P_angle = 0.9;//�Ƕȵ���Pϵ��
		double distol = 0.01;


		if ((InRange(XPos, distol, Pos[x]) && (InRange(YPos, distol, Pox[y])) {
			if (InRange(ThisFacingAngle, angletol, FinalFacingAngle)) {
				return true;
			}
			else {
				myRobot.Drive(0, -(FinalFacingAngle - ThisFacingAngle)*P_angle)
			}

		}
		else {
			x_dif = Pox[x] - XPos;
			y_dif = Pox[y] - YPos;
			TargetAngle = atan(x_dif / y_dif);
			if (InRange(ThisFacingAngle, angleact, TargetAngle)) {
				myRobot.Drive(MovingSpeed, -(TargetAngle - ThisFacingAngle)*P)
			}
			else {
				myRobot.Drive(0, -(TargetAngle - ThisFacingAngle)*P)
			}
		}

		return false;
	}
	bool InRange(double input, double tolerate, double target) {
		return ((input >= target - tolerate) && (input <= target + tolerate));
	}

	///̧����
	void LeftSolenoid() {
		BotCogSolenoid.Set(false);
		UpCogSolenoid.Set(false);
		CogFriWheel.Set(0);
		BotSolenoidDown = false;
		UpSolenoidDown = false;
	}

	///ͼ���㷨
	double err = 0;

	void ReadFromRaspberryPi() {
		err = round(rand() * 100);//��ʱʹ�����������Ӧ��ȡ��ݮ��ͼ������!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	}

	int AngleCallibrate() {
		//��ȡ���

		err += AngleSliderCoe*(stick.GetZ() - 0.5);
		if (abs(err)>AngleTolerate) {
			AnglePosition -= err*AngleCoe;
		}
		else {
			Calibrated = true;
		}
		return AnglePosition;
	}

	///����
	void ShootBall(double speed) {
		double ShootingSpeed;
		BallTransferBelt.Set(speed);
		BallTransferWheel.Set(speed);
		ShootWheel.Set(CalculateShootingSpeed());
	}

	double CalculateShootingSpeed() {
		//��������
		double distance = 0;
		double height = 3.4;
		double result = 0;
		double AirFriCoe = 0.02;
		double a = 2;
		double g = 9.8;

		distance = sqrt(pow((BallPos[x] - Pos[x]), 2) + pow((BallPos[y] - Pos[y]), 2));
		result = pow(distance, 2)*AirFriCoe + sqrt(2 * pow(cos(a), 2) / ((g*pow(distance, 2))*(tan(a)*distance - height));
		return result;
	}



};

START_ROBOT_CLASS(Robot)
