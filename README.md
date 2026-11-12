# CrazyArcade-SFML
C++ SFML Crazy Arcade-style 2-player game

크레이지 아케이드 스타일의 2인용 물풍선 대전 게임입니다.
모블 국비과정 C++ 팀 프로젝트(3팀, 5인, 2주)로 만들었습니다.

## 기술 스택
- C++ / SFML 3.1
- Visual Studio 2022

## 팀 역할 분담
| 파트 | 내용 |
|---|---|
| A | 맵·블록 (`TileMap`, `TileArt`) |
| **B (조민경)** | **플레이어 (`Player`)** |
| C | 물풍선·폭발 (`WaterBalloon`) |
| D | 아이템·UI (`Item`, `UI`) |
| E | 게임 진행·화면 (`Game`) |

## 내 역할 — 플레이어 파트
- 1P / 2P 키 입력 분리 및 동시 조작
- 타일 기반 이동
- 방향별 걷기 애니메이션, 캐릭터 출력

## 실행 방법
1. SFML 3.1을 `C:\SFML-3.1.0`에 설치 (경로가 다르면 프로젝트 속성의 include/lib 경로 수정)
2. `CrazyArcade.sln` 열고 **x64 / Debug**로 빌드
3. `C:\SFML-3.1.0\bin`의 `sfml-*-d-3.dll` 파일을 실행 파일 폴더에 복사 후 실행
