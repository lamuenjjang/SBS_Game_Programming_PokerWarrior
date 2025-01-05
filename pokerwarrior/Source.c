#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>

#pragma region 변수 선언
// 능력치 증가 횟수 변수
static int increase_done = 3;
// 방어 선택 여부 확인 변수
static int player_defended = 0;
// 이벤트 제어 변수
static int event = -1;
// 이벤트 진행 턴수를 설정합니다.
static int event_ing = 0;
// 현재 진행되고 있는 턴
static int turns = 1;
#pragma endregion

#pragma region 기본 함수 선언
// 화면에서 커서를 제거하는 함수입니다
void CursorView()
{
	CONSOLE_CURSOR_INFO cursorinfo = { 0, };
	cursorinfo.dwSize = 1;
	cursorinfo.bVisible = FALSE;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorinfo);
}

void SetColor(int textcolor) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), textcolor);
}

void gotoxy(int x, int y) {
	COORD Pos;
	Pos.X = x;
	Pos.Y = y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), Pos);
}
#pragma endregion

#pragma region 열거형 및 구조체 선언

// 스페이드, 다이아, 하트, 클로버 열거형 상수입니다
enum shape
{
	spade,
	dia,
	heart,
	clover
};

// 에이스, 잭,퀸,킹의 열거형 상수입니다,
enum specialcard
{
	Ace = 1,
	Jack = 11,
	Queen,
	King
};

// 카드의 정보를 저장하는 구조체입니다.
struct card
{
	int number;
	int shape;
};

// 포커족보에 대한 열거형 상수입니다.
enum Jokbo {
	Nopair,
	Onepair,
	Twopair,
	Triple,
	Straight,
	BackStraight,
	Mountain,
	Flush,
	Fullhouse,
	Fourcard,
	StraightFlush,
	BackStraightFlush,
	RoyalStraightFlush
};

// 플레이어의 정보를 저장하는 구조체입니다.
struct Player
{

	int hp;

	float atk;
	float def;
	float cri;

	struct card p_card[5];
};

// 몬스터의 정보를 저장하는 구조체입니다.
struct monster
{
	int hp;
	float atk;
	
	int turns;

	int next;
};

struct card Card[52];

#pragma endregion

#pragma region 포커 관련 함수 선언

// 셔플 함수를 사용해 덱을 섞습니다.
void Shuffle()
{
	// 증가 횟수가 끝나지 않는다면, 셔플을 실행
	if (increase_done > 0) {
		// 초기화 할때 난수를 무작위로 생성해줍니다.
		srand(time(NULL));

		// deck 구조체 배열의 0번째 인덱스에 있는 값과,
		// rand()함수를 통해 랜덤한 인덱스에 있는 값을 섞음.
		for (int i = 0; i < 52; i++)
		{
			int rindex = rand() % 52;
			struct card tempcard = Card[rindex];
			Card[rindex] = Card[0];
			Card[0] = tempcard;
		}
	}
	
}

// deck 구조체 배열을 선언한 후, 배열의 인덱스에 따라 문양을 구분
// 0 ~ 12 : 스페이드
// 13 ~ 25 : 다이아
// 26 ~ 38 : 하트
// 39 ~ 51 : 클로버

// filldeck 함수로 덱의 값을 채움.
// 13으로 나눈 몫으로 문양을 구분, 13으로 모듈러 연산 후 나온 나머지 + 1의 값으로 숫자를 구분 
void FillDeck() {
	for (int i = 0; i < 52; i++)
	{
		Card[i].shape = i / 13;
		Card[i].number = i % 13 + 1;
	}
}

// 카드의 위치를 바꾸어 주는 함수입니다.
void Swap(struct card* card1, struct card* card2) {
	struct card temp = *card1;
	*card1 = *card2;
	*card2 = temp;
}

// 버블정렬로 카드를 오름차순으로 정리하는 함수입니다.
void bubble(struct card* cardlist) {
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 4 - i; j++) {
			if (cardlist[j].number > cardlist[j + 1].number) {
				Swap(&cardlist[j], &cardlist[j + 1]);
			}
		}
	}
}

// 플러쉬를 확인하는 함수입니다.
int is_flush(struct card* cardlist) {
	int is_flush = 1;
	for (int i = 0; i < 4; i++) {
		for (int j = i + 1; j < 5; j++) {
			if (cardlist[i].shape != cardlist[j].shape) {
				return 0;
			}
		}
	}

	return 25;
}

// 페어를 확인하는 함수입니다.
int is_pair(struct card* cardlist) {

	// 노페어 = 0
	// 원페어 = 1
	// 투페어 = 2
	// 트리플 = 3
	// 포카드 = 6
	int cnt = 0;
	for (int i = 0; i < 4; i++) {
		for (int j = i + 1; j < 5; j++) {
			if (cardlist[i].number == cardlist[j].number) {
				cnt++;
			}
		}
	}
	return cnt;
}

// 마운틴 (무늬 다른 10,J,Q,K,A)를 확인하는 함수입니다.
int is_mountain(struct card* cardlist) {
	int tmp = 0;
	for (int i = 0; i < 5; i++) {
		if (cardlist[i].number == 9 + i || cardlist[i].number == 1) {
			tmp++;
		}
	}
	if (tmp == 5)
		return 20;
	else
		return 0;
}

// 백스트레이트 (무늬 다른 1,2,3,4,5)를 확인하는 함수입니다.
int is_Back(struct card* cardlist) {
	int tmp = 0;
	for (int i = 0; i < 5; i++) {
		if (cardlist[i].number == 1 + i) {
			tmp++;
		}
	}
	if (tmp == 5)
		return 15;
	else
		return 0;
}

// 스트레이트 (연속된 5개의 숫자)
int is_Straight(struct card* cardlist) {
	int tmp = 0;
	for (int i = 0; i < 4; i++) {
		if (cardlist[i + 1].number - cardlist[i].number == 1) {
			tmp++;
		}
	}
	if (tmp == 4)
		return 10;
	else
		return 0;
}

// 5장의 카드의 족보를 체크하는 함수입니다.
int checkJokbo(struct card* deck) {
	// 함수를 통해 얻은 값을 다 더해서 나온 값에 따라 0 ~ 12의 족보 값을 리턴
	// 0 :  HIGHCARD (족보 없음)
	// 1 :  원페어
	// 2 :  투페어
	// 3 :  트리플
	// 10 : 스트레이트
	// 15 :  백스트레이트
	// 20:  마운틴
	// 25 : 플러쉬
	// 4 :  풀하우스
	// 6 :  포카드
	// 35 : 스티플
	// 50 : 백스티플
	// 45 : 로티플
	int jokbo = is_flush(deck) + is_pair(deck) + is_Back(deck) + is_mountain(deck) + is_Straight(deck);

	switch (jokbo)
	{
	case 0: return 0;
		break;
	case 1: return 1;
		break;
	case 2: return 2;
		break;
	case 3: return 3;
		break;
	case 10: return 4;
		break;
	case 15: return 5;
		break;
	case 20: return 6;
		break;
	case 25: return 7;
		break;
	case 4: return 8;
		break;
	case 6: return 9;
		break;
	case 35: return 10;
		break;
	case 50: return 11;
		break;
	case 45: return 12;
		break;
	}

}

void Cutdeck(struct card* deck, struct Player *p) {
	for (int i = 0; i < 5; i++) {
		p->p_card[i] = deck[i];
	}
}

#pragma endregion

#pragma region 이벤트 함수

// 이벤트 1 : 몬스터가 공격까지 남은 턴을 비공개 처리합니다.
// 5턴 지속
void UnknownTurns(struct monster *m) {
	gotoxy(52, 7);
	SetColor(4);
	printf(" 적의 의도를 알수 없다..");
	SetColor(15);
	gotoxy(52, 8);
	printf(" 5턴동안 적의 남은 턴을");
	gotoxy(52, 9);
	printf(" 알수 없습니다.(무작위)");			
	
	event = 0;
	event_ing = 5;
	m->turns = rand() % 3;
}

void hideTurns()
{
	if (event_ing > 0 && event == 0) {
		gotoxy(38, 6);
		printf("? ? ? ? ? ? ");
	}
	else if (event_ing == 0)
		event = -1;
}

// 이벤트 2 : 몬스터의 공격력이 2배가 됩니다.
// 턴 즉시 발동
void TwiceMatk(struct monster* m) {
	
	m->atk = m->atk * 2;
	gotoxy(52, 7);
	SetColor(5);
	printf(" 슬라임이 분노했다!");
	SetColor(15);
	gotoxy(52, 8);
	printf(" 슬라임의 공격력이 두배가");
	gotoxy(52, 9);
	printf(" 됩니다.");

	event_ing = 1;
	event = 1;
}

// 이벤트 3 : 플레이어의 공격력이 절반이 됩니다.
// 턴 즉시 발동
void HalfAtk(struct Player* p) {
	

	p->atk = p->atk / 2;

	gotoxy(52, 7);
	SetColor(12);
	printf(" 슬라임의 부식침을 맞았다!");
	SetColor(15);
	gotoxy(52, 8);
	printf(" 플레이어의 공격력이");
	gotoxy(52, 9);
	printf(" 절반이 됩니다.");

	event_ing = 1;
	event = 2;
}

// 턴 즉시 발동 함수들을 제어하는 함수입니다.
void not_ing_event_end() {
	if (event == 1 || event == 2 || event == 3) {
		event = -1;
	}
}

// 이벤트 4 : 몬스터가 체력을 50 회복합니다.
void RegenHP(struct monster * m) {
	event = 3;
	m->hp = m->hp + 50;
	if (m->hp > 500)
		m->hp = 500;
	gotoxy(52, 7);
	SetColor(3);
	printf(" 슬라임이 자연치유하였다.");
	SetColor(15);
	gotoxy(52, 8);
	printf(" 슬라임이 체력을 50");
	gotoxy(52, 9);
	printf(" 회복합니다.");
}

// 이벤트 5 : 플레이어의 능력치 강화 횟수가 0회가 됩니다.
// 3턴 지속
void NOIncrease() {
	
	gotoxy(52, 7);
	SetColor(9);
	printf(" 슬라임의 저주에 걸렸다..");
	SetColor(15);
	gotoxy(52, 8);
	printf(" 능력치 강화 횟수가 3턴");
	gotoxy(52, 9);
	printf(" 동안 0회가 됩니다.");

	event = 4;
	event_ing = 3;
}

void ZeroIncrease() {
	if (event_ing > 0 && event == 4) {
		increase_done = 0;
	}
	else if (event_ing == 0)
		event = -1;
}
#pragma endregion

#pragma region 전투 관련 함수

// 치명타 확률에 따른 치명타 여부를 반환
// 2 = 치명타 O, 1 = 치명타 X 
int is_Critical(int Cri) {
	srand(time(NULL));

	int tmp = rand() % 100;

	if (tmp < Cri) {
		return 2;
	}
	else
		return 1;
}

// 플레이어 공격시 호출하는 함수
void PlayerAttack(struct Player*p, struct monster*m) {
	int tmp;
	int Critical = is_Critical(p->cri);
	tmp = p->atk * Critical;
	m->hp = (int)m->hp - tmp;
	// 능력치 증가 횟수 초기화
	increase_done = 3;
	// 몬스터의 턴 감소
	m->turns--;

	gotoxy(52, 5);
	if (Critical == 1) {
		printf(" %d 의 피해를 주었습니다.", tmp);
	}
	else if (Critical == 2) {
		SetColor(4);
		printf(" %d 의 치명타를 입혔습니다.", tmp);
		SetColor(15);
	}
	gotoxy(52, 6);
	printf(" 상대의 남은 체력 %d", m->hp);
}							

// 몬스터 공격시 호출하는 함수
void MonsterAttack(struct monster *m, struct Player* p,int defend) {
	int tmp;
	if (defend == 0) {
		tmp = (int)m->atk;
		p->hp = p->hp - tmp;
	}
	else if (defend == 2) {
		tmp = m->atk  - p->def;
	
		if (tmp > 0) {
			p->hp = p->hp - tmp;
		}
	}
	m->turns = 4;
	gotoxy(52, 5);
	printf(" %d 의 피해를 입었습니다", tmp);
	gotoxy(52, 6);
	printf(" 당신의 남은 체력 %d", p->hp);
}

// 플레이어 방어시 방어했음을 리턴하는 함수
int Defend(struct monster* m) {
	m->turns--;
	increase_done = 3;
	return 2;
}

// 공격력 증가 함수
void IncreaseAtk(int jokbo,struct Player *p) {
	if (increase_done > 0) {
		switch (jokbo) {
		case Nopair: p->atk = p->atk + 0.1;
			break;
		case Onepair: p->atk = p->atk + 0.5;
			break;
		case Twopair: p->atk = p->atk + 1;
			break;
		case Triple: p->atk = p->atk + 1.5;
			break;
		case Straight: p->atk = p->atk + 3;
			break;
		case BackStraight: p->atk = p->atk + 4;
			break;
		case Mountain: p->atk = p->atk + 4.5;
			break;
		case Flush: p->atk = p->atk + 5;
			break;
		case Fullhouse: p->atk = p->atk + 7.5;
			break;
		case Fourcard: p->atk = p->atk + 10;
			break;
		case StraightFlush: p->atk = p->atk * 2;
			break;
		case BackStraightFlush: p->atk = p->atk * 4;
			break;
		case RoyalStraightFlush: p->atk = p->atk * 8;
			break;
		}
		increase_done--;
	}
	
}

// 방어력 증가 함수
void IncreaseDef(int jokbo, struct Player* p) {
	if (increase_done > 0) {
		switch (jokbo) {
		case Nopair: p->def = p->def + 0.1;
			break;
		case Onepair: p->def = p->def + 0.5;
			break;
		case Twopair: p->def = p->def + 1;
			break;
		case Triple: p->def = p->def + 1.5;
			break;
		case Straight: p->def = p->def + 3;
			break;
		case BackStraight: p->def = p->def + 4;
			break;
		case Mountain: p->def = p->def + 4.5;
			break;
		case Flush: p->def = p->def + 5;
			break;
		case Fullhouse: p->def = p->def + 7.5;
			break;
		case Fourcard: p->def = p->def + 10;
			break;
		case StraightFlush: p->def = p->def * 2;
			break;
		case BackStraightFlush: p->def = p->def * 4;
			break;
		case RoyalStraightFlush: p->def = p->def * 8;
			break;
		}
		increase_done--;
	}
}

// 치몀타 확률 증가 함수
void IncreaseCri(int jokbo, struct Player* p) {
	if (increase_done > 0) {
		switch (jokbo) {
			case Nopair: p->cri = p->cri + 0.1;
				break;
			case Onepair: p->cri = p->cri + 0.5;
				break;
			case Twopair: p->cri = p->cri + 1;
				break;
			case Triple: p->cri = p->cri + 1.5;
				break;
			case Straight: p->cri = p->cri + 3;
				break;
			case BackStraight: p->cri = p->cri + 4;
				break;
			case Mountain: p->cri = p->cri + 4.5;
				break;
			case Flush: p->cri = p->cri + 5;
				break;
			case Fullhouse: p->cri = p->cri + 7.5;
				break;
			case Fourcard: p->cri = p->cri + 10;
				break;
			case StraightFlush: p->cri = p->cri * 2;
				break;
			case BackStraightFlush: p->cri = p->cri * 4;
				break;
			case RoyalStraightFlush: p->cri = p->cri * 8;
				break;
			}
		increase_done--;
	}
}
#pragma endregion

#pragma region UI 함수 선언
// GAMEMAP() 함수를 통해서 인터페이스 제작
void GameMap()
{
	gotoxy(0, 0);
	for (int i = 0; i < 80; i++) {
		printf("=");
	}
	printf("\n");
	for (int i = 1; i < 10; i++) {
		gotoxy(0, i);
		printf("||");
		gotoxy(50, i);
		printf("||");
		gotoxy(78, i);
		printf("||");
		printf("\n");
	}
	for (int i = 0; i < 50; i++) {
		printf("=");
	}
	printf("||");
	gotoxy(78, 10);
	printf("||");
	gotoxy(52, 3);
	for (int i = 0; i < 26; i++) {
		printf("=");
	}
	gotoxy(52, 4);
	printf("<로그>");
	gotoxy(52, 10);
	for (int i = 0; i < 26; i++) {
		printf("=");
	}
	printf("\n");

	for (int i = 11; i < 18; i++) {
		gotoxy(0, i);
		printf("||");
		gotoxy(50, i);
		printf("||");
		gotoxy(78, i);
		printf("||");
		printf("\n");
	}
	for (int i = 0; i < 80; i++) {
		printf("=");
	}
}

// 플레이어에게 제공된 포커 카드의 정보를 제시
void Pokercard(struct card* playercard) {
	gotoxy(4, 11);
	printf("<카드>");
	int card = 0;

	gotoxy(4, 12);
	printf("┌━━━┬━━━┬━━━┬━━━┬━━━┐\n");
	gotoxy(4, 13);
	do
	{
		printf("│");
		if (playercard->shape == dia || playercard->shape == heart) SetColor(4);
		switch (playercard->shape)
		{
		case spade: printf("♠");
			break;
		case dia: printf("◆");
			break;
		case heart: printf("♥");
			break;
		case clover: printf("♣");
			break;
		}
		switch (playercard->number)
		{
		case Ace: printf(" A");
			break;
		case King: printf(" K");
			break;
		case Jack: printf(" J");
			break;
		case Queen: printf(" Q");
			break;
		case 10:printf("10");
			break;
		default: printf(" %d", playercard->number);
			break;
		}
		SetColor(15);
		card++;
		playercard++;
	} while (card < 5);
	printf("│");
	printf("\n");
	gotoxy(4, 14);
	printf("└━━━┴━━━┴━━━┴━━━┴━━━┘");
	
}

// 족보 정보를 제시
void ShowJokbo(struct card* playercard) {
	gotoxy(4, 15);
	switch (checkJokbo(playercard))
	{
	case Nopair:
		SetColor(8);
		printf("노페어");
		gotoxy(4, 16);
		printf("+0.1%");
		break;
	case Onepair:
		SetColor(7);
		printf("원페어");
		gotoxy(4, 16);
		printf("+0.5%");
		break;
	case Twopair:
		SetColor(3);
		printf("투페어");
		gotoxy(4, 16);
		printf("+1.0%");
		break;
	case Triple: 
		SetColor(3);
		printf("트리플");
		gotoxy(4, 16);
		printf("+1.5%");
		break;
	case Straight: 
		SetColor(10);
		printf("스트레이트");
		gotoxy(4, 16);
		printf("+3.0%");
		break;
	case BackStraight:
		SetColor(10);
		printf("백스트레이트");
		gotoxy(4, 16);
		printf("+4.0%");
		break;
	case Mountain:
		SetColor(10);
		printf("마운틴");
		gotoxy(4, 16);
		printf("+4.5%");
		break;
	case Fullhouse: 
		SetColor(13);
		printf("풀하우스");
		gotoxy(4, 16);
		printf("+5.0%");
		break;
	case Flush: 
		SetColor(13);
		printf("플러쉬");
		gotoxy(4, 16);
		printf("+7.5%");
		break;
	case Fourcard: 
		SetColor(13);
		printf("포카드");
		gotoxy(4, 16);
		printf("+10.0%");
		break;
	case StraightFlush: 
		SetColor(6);
		printf("스티플");
		gotoxy(4, 16);
		printf("x 2");
		break;
	case BackStraightFlush:
		SetColor(6);
		printf("백스티플");
		gotoxy(4, 16);
		printf("x 4");
		break;
	case RoyalStraightFlush:
		SetColor(4);
		printf("로티플");
		gotoxy(4, 16);
		printf("x 8");
		break;
	}
	SetColor(15);
}

void GrowUI() {
	gotoxy(30, 11);
	printf("<능력치 증가>");
	gotoxy(30, 12);
	printf("┌━━━━━━━━━━━━━━━━━┐");
	gotoxy(30, 13);
	printf("│A : 공격력 증가  │");
	gotoxy(30, 14);
	printf("│D : 방어력 증가  │");
	gotoxy(30, 15);
	printf("│C : 치명확률 증가│");
	gotoxy(30, 16);
	printf("└━━━━━━━━━━━━━━━━━┘");
}

// 플레이어 스프라이트 이미지를 출력하는 함수
void Pview() {

	gotoxy(8, 5);
	printf("●");
	gotoxy(7, 6);
	printf("⊙┃→");
	gotoxy(8, 7);
	printf("Λ");
}

// 몬스터의 스프라이트 이미지를 출력하는 함수
void Slimeview() {
	gotoxy(28, 4);
	printf(" ___");
	gotoxy(26, 5);
	printf(" ／   ＼");
	gotoxy(26, 6);
	printf("／     ＼");
	gotoxy(26, 7);
	printf("│ㅇㅅㅇ │");
	gotoxy(26, 8);
	printf("＼_____／");
}

/*void Spiderview() {
	gotoxy(28, 4);
	printf(".-,  ,,  ,-. ");
	gotoxy(28, 5);
	printf(" _ \\(::)/ _ ");
	gotoxy(28, 6);
	printf("   ( '.3  E.' )");
	gotoxy(28, 7);
	printf("   /'__'\\ ");
	gotoxy(28, 8);
	printf("  (      ) ");
}*/

void P_status(struct Player player) {
	gotoxy(8, 2);
	if (player.hp < 10) {
		SetColor(4);
		printf("hp = %d  / 100", player.hp);
	}
	else if (player.hp < 50){
		SetColor(6);
		printf("hp = %d / 100", player.hp);
	}
	else {
		SetColor(10);
		printf("hp = %d / 100", player.hp);
	}
	SetColor(15);
	gotoxy(11, 5);
	printf("Atk: %.2f", player.atk);
	gotoxy(11, 6);
	printf("Def: %.2f", player.def);
	gotoxy(11, 7);
	printf("Cri: %.2f", player.cri);
	

}

void M_status(struct monster monster) {
	gotoxy(30, 2);
	if (monster.hp < 10) {
		SetColor(4);
		printf("hp = %d  / 500", monster.hp);
	}
	else if(monster.hp < 100) {
		SetColor(6);
		printf("hp = %d / 500", monster.hp);
	}
	else {
		SetColor(10);
		printf("hp = %d / 500", monster.hp);
	}
	SetColor(15);
	gotoxy(38, 5);
	printf("Atk: %.2f", monster.atk);
	gotoxy(38, 6);
	if (monster.turns > 0)
		printf("%d turn left", monster.turns);
	else if (monster.turns == 0) {
		SetColor(12);
		printf("now Attack!!");
		SetColor(15);
	}
		
	
}

void increaseUI(int increase) {
	gotoxy(52,1);
	printf(" Increase ability");
	if (increase > 0)
	{
		printf(" (%d / 3)", increase);
	}
	else if (increase == 0) {
		SetColor(4);
		printf(" (0 / 3)");
		SetColor(15);
	}
}

void KeyUI() {
	gotoxy(52, 11);
	printf("<키>");
	gotoxy(52, 12);
	printf("┌━━━━━━━━━━┐");
	gotoxy(52, 13);
	printf("│ K : 공격 │");
	gotoxy(52, 14);
	printf("│ L : 방어 │");
	gotoxy(52, 15);
	printf("└━━━━━━━━━━┘");
}

void nowTurns(int turns) {
	gotoxy(2, 1);
	SetColor(8);
	printf("{ %d turn }", turns);
	SetColor(15);
}

void Victoryscene() {
	gotoxy(0, 0);
	for (int i = 0; i < 30; i++) {
		printf("=");
	}
	for (int i = 1; i < 8; i++) {
		gotoxy(0, i);
		printf("||");
		gotoxy(28, i);
		printf("||");
	}
	printf("\n");
	for (int i = 0; i < 30; i++) {
		printf("=");
	}
	gotoxy(2, 1);
	printf("You Win this Game");
	gotoxy(2, 2);
	printf("Thx for playing");
	gotoxy(2, 4);
	printf("This game is DEMO ver");
}

void Gameoverscene() {
	gotoxy(0, 0);
	for (int i = 0; i < 30; i++) {
		printf("=");
	}
	for (int i = 1; i < 8; i++) {
		gotoxy(0, i);
		printf("||");
		gotoxy(28, i);
		printf("||");
	}
	printf("\n");
	for (int i = 0; i < 30; i++) {
		printf("=");
	}
	gotoxy(2, 1);
	printf("GAME OVER");
	gotoxy(2, 2);
	printf("pls Quit this console,");
	gotoxy(2, 3);
	printf("if you want to play again");
	gotoxy(2, 5);
	printf("This game is DEMO ver");
}
#pragma endregion





int main()
{
#pragma region 기본 함수 및 덱 셔플 함수 호출
	srand(time(NULL));
	CursorView();
	FillDeck();
	Shuffle();
#pragma endregion

	

	system("mode con cols=80 lines=30");

	struct Player _player = {100,1.0f,1.0f,10.0f};
	struct monster Slime = {500,10.0f,4};

	Cutdeck(&Card, &_player);

	while (_player.hp > 0 && Slime.hp > 0) {
#pragma region 포커 함수 호출
		Shuffle();
		Cutdeck(&Card, &_player);
		bubble(&_player.p_card);
#pragma endregion

#pragma region 이벤트 제어

		if (turns % 5 == 0 && event == -1) {
			int randomEvent = rand() % 5;

			switch (randomEvent)
			{
			case 0:
				UnknownTurns(&Slime);
				break;
			case 1:
				TwiceMatk(&Slime);
				break;
			case 2:
				HalfAtk(&_player);
				break;
			case 3:
				RegenHP(&Slime);
				break;
			case 4:
				NOIncrease();
				break;
			}
		}

		

#pragma endregion

#pragma region 플레이어 & 몬스터 UI 호출
		Pview();
		P_status(_player);
		if (Slime.hp > 0)
		{
			Slimeview();
			M_status(Slime);
		}
		hideTurns();
#pragma endregion

#pragma region UI 함수 호출
		Pokercard(&_player.p_card);
		ShowJokbo(_player.p_card);
		GameMap();

		GrowUI();
		ZeroIncrease();
		increaseUI(increase_done);
		KeyUI();
		nowTurns(turns);
#pragma endregion
		
		char key = _getch();
		system("cls");
		
		// A/D/C/K/L 키 값 입력시 switch 문의 각 케이스가 실행됩니다.
		 switch (key) {
		 case 97	: IncreaseAtk(checkJokbo(_player.p_card),&_player);
			 break;
		 case 100	: IncreaseDef(checkJokbo(_player.p_card), &_player);
			 break;
		 case 99	: IncreaseCri(checkJokbo(_player.p_card), &_player);
			 break;
		 case 107	: PlayerAttack(&_player, &Slime);
			 turns++;
			 event_ing--;
			 not_ing_event_end();
			 break;
		 case 108: player_defended = Defend(&Slime);
			 turns++;
			 event_ing--;
			 not_ing_event_end();
			 break;
		 }
	
		 // Slime의 턴이 0보다 작아지면 공격합니다.
		 if (Slime.turns < 0) {
			 MonsterAttack(&Slime, &_player, player_defended);
			 player_defended = 0;
		 }
		 
	}

	// 플레이어 또는 몬스터의 체력이 0보다 작아지면
	// 승리 화면 또는 패배 화면을 출력합니다.
	system("cls");
	if (Slime.hp < 0) {
		Victoryscene();
	}
	else if (_player.hp < 0) {
		Gameoverscene();
	}
	while(1) {
	}
}