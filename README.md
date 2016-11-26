# smart_auth
윈도우 로그온에서 기존에 존재하던 패스워드 인증 뿐만 아니라 다른 여러가지 인증을 복합적으로 사용할 수 있게 도와주는 솔루션

## 요구사항
플랫폼 : 윈도우 7 이상
의존성 : 의존성이 없게 설계 되었으므로 어떠한 설치도 필요하지 않습니다

## 구성
ui : 인증요소를 추가/삭제할 수 있고 옵션을 선택할 수 있는 electron 프로그램

donglein 폴더 : donglein 인증요소에 관한 파일
google_otp 폴더 : google_otp 인증요소에 관한 파일
smart_id_card 폴더 : smart_id_card 인증요소에 관한 파일

SmartAuthCredentialProvider : 크리덴셜 프로바이더에 관한 파일

project_brochure : 프로젝트 브로셔 제작에 필요한 파일
project_panel : 프로젝트 패널 제작에 필요한 파일
presentation : 프로젝트 발표에 필요한 파일

weekly_reports : 주간 보고서 파일

## 프로그램의 실행에 관한 특징
이 프로그램이 실행되면 다음과 같은 파일들이 옮겨집니다.
- SmartAuthCredentialProvider.dll -> windows/system32 폴더
- donglein_driver.sys -> windows/inf 폴더
- donglein_driver.inf -> windows/inf 폴더
- WdfCoinstaller01011.dll -> windows/inf 폴더

이 프로그램이 실행되면 HKEY_LOCAL_MACHINE/SOFTWARE/SmartAuth 키를 만들고 그 하위에 정보를 저장합니다

donglein 인증요소를 사용할 시에는 일반 usb가 필요합니다
smart_id_card 인증요소를 사용할 시에는 스마트 사원증과 아두이노가 필요합니다.
