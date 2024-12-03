#include "TCPServer.h"
//#include "Player.h"


// Define static member variables

CRITICAL_SECTION consoleCS; // CriticalSection for console output
UpdateInfo TCPServer::updateInfo[2];          // 클라이언트 업데이트 정보 정의
RenderInfo TCPServer::renderInfo[2];          // 클라이언트 렌더링 정보 정의
PlayerInfo TCPServer::playerinfo[2];          // 클라이언트 렌더링 정보 정의

void SetCursorPosition(int x, int y) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD position = { (SHORT)x, (SHORT)y };
	SetConsoleCursorPosition(hConsole, position);
}

TCPServer::TCPServer() {
	// Create player
	for (int i = 0; i < 2; ++i) {
		players.push_back(new Player(100, 200, 40, 20, 0));
	}
	InitializeCriticalSection(&consoleCS);
	// Create and initialize enemies
	for (int i = 0; i < 14; ++i) {
		NM_zombie* zombie;
		if(i == 0)
			zombie = new NM_zombie(600, 1200, 20, 30, 27, 일반);
		else
			zombie = new NM_zombie(1200, 1200, 20, 30, 27, 일반);
		//zombie->setPlayer(players); // Player 객체 전달
		enemyList.push_back(zombie);

	}
	m_hUpdateEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}
TCPServer::~TCPServer() {
	// Delete player

	DeleteCriticalSection(&consoleCS);

	// Close all sockets and threads
	for (SOCKET& client : client_sockets) {
		closesocket(client);
	}
	for (HANDLE& thread : client_threads) {
		CloseHandle(thread);
	}
	WSACleanup();
}
void TCPServer::BindAndListen() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cout << "WSAStartup failed." << std::endl;
		exit(1);
	}

	listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_sock == INVALID_SOCKET) {
		std::cout << "Socket creation failed." << std::endl;
		exit(1);
	}

	sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	//serverAddr.sin_addr.s_addr = INADDR_ANY;      // Bind to all available interfaces
	serverAddr.sin_port = htons(Portnum);
	//serverAddr.sin_port = htons(9000);

	char hostName[256];
	gethostname(hostName, sizeof(hostName));
	std::cout << hostName << std::endl;

	hostent* ptr = gethostbyname(hostName);
	memcpy(&serverAddr.sin_addr, ptr->h_addr_list[0], ptr->h_length);

	//const char* ppppp = "121.190.132.246";
	//inet_pton(AF_INET, ppppp, &serverAddr.sin_addr);

	char iptest[22];

	inet_ntop(AF_INET, &serverAddr.sin_addr, iptest, sizeof(iptest));
	printf("서버의 ip주소 - %s\n", iptest);

	if (bind(listen_sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		std::cout << "Binding failed" << std::endl;
		closesocket(listen_sock);
		WSACleanup();
		exit(1);
	}

	if (listen(listen_sock, SOMAXCONN) == SOCKET_ERROR) {
		std::cout << "Listening failed" << std::endl;
		closesocket(listen_sock);
		WSACleanup();
		exit(1);
	}

}
void TCPServer::Execute() {
	BindAndListen();
	std::cout << "Waiting for clients to connect..." << std::endl;
	AcceptClients();

	::QueryPerformanceFrequency((LARGE_INTEGER*)&m_nQueryPerfomancFrequency);
	float timeScale = 1.0f / m_nQueryPerfomancFrequency;

	::QueryPerformanceCounter((LARGE_INTEGER*)&m_nLastTime);
	float fElapsedTime;
	while (true) {
		::QueryPerformanceCounter((LARGE_INTEGER*)&m_nCurrentTime);
		fElapsedTime = (m_nCurrentTime - m_nLastTime) * timeScale;		// 
		while (fElapsedTime < (1.0f / 60.0f)) {
			::QueryPerformanceCounter((LARGE_INTEGER*)&m_nCurrentTime);
			fElapsedTime = (m_nCurrentTime - m_nLastTime) * timeScale;
		}
		m_nLastTime = m_nCurrentTime;
		// Wait for all threads to signal their events
		//WaitForMultipleObjects(clientCount, client_events.data(), TRUE, INFINITE);
		// Update game state

		ResetEvent(m_hUpdateEvent);
		Update();
		SetEvent(m_hUpdateEvent);
	}
}
void TCPServer::AcceptClients() {
	sockaddr_in clientaddr;
	int addrlen = sizeof(clientaddr);

	while (clientCount < 2) {
		SOCKET clientSocket = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);

		if (clientSocket == INVALID_SOCKET) {
			std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
			continue;
		}
		cout << "클라이언트[" << clientCount + 1 << "]" << "가 입장했습니다" << endl;
		/*std::cout << "Client connected. Address: " << inet_ntoa(clientaddr.sin_addr)
			<< ", Port: " << ntohs(clientaddr.sin_port) << std::endl;*/


		client_sockets.push_back(clientSocket);
		/*HANDLE clientEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		client_events.push_back(clientEvent);*/
		//client_events.push_back(CreateEvent(NULL, FALSE, FALSE, NULL));
		//m_hUpdateEvent.push_back(CreateEvent(NULL, TRUE, FALSE, NULL));

		// Prepare ThreadArg
		ThreadArg* data = new ThreadArg{ client_sockets[clientCount], clientCount, this };

		// Pass the ThreadArg to the thread
		HANDLE clientThread = CreateThread(NULL, 0, ClientThread, data, 0, NULL);

		client_threads.push_back(clientThread);
		++clientCount;
	}

	//hReadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	hWriteEvent = CreateEvent(NULL, FALSE, TRUE, NULL);

	closesocket(listen_sock);
	/*EnterCriticalSection(&consoleCS);
	SetCursorPosition(0, 5);
	std::cout << "Two clients connected. No longer accepting new connections." << std::endl;
	LeaveCriticalSection(&consoleCS);*/
}


void TCPServer::Update() {
	/*EnterCriticalSection(&consoleCS);
	SetCursorPosition(0, 2);
	std::cout << "Updating game state..." << std::endl;
	LeaveCriticalSection(&consoleCS);*/

	// Update enemy positions
	for (int i = 0; i < enemyList.size(); ++i) {
		NM_zombie* zombie = dynamic_cast<NM_zombie*>(enemyList[i]);
		zombie->setPlayer(players);
		if (!zombie->Death_check()) {  // Check if the zombie is alive
			zombie->walk_ani(enemyList.data(), i); // Pass parameters to `walk_ani`
		}
		else {
			// Revive the zombie if it's dead

			zombie->Revive(); // Move to a random location
		}
		zombie->UpdateMatrix();
	}


	// Exchange player info between clients
	if (clientCount == 2) {
		renderInfo[0].opposite = { updateInfo[1].cameraEYE, updateInfo[1].cameraangle };
		renderInfo[1].opposite = { updateInfo[0].cameraEYE, updateInfo[0].cameraangle };
	}

	//=========================================================start Dong-ki-wow~



	{// Print current state to console
	/*EnterCriticalSection(&consoleCS);
	for (int i = 0; i < clientCount; ++i) {
		SetCursorPosition(0, 6 + i * 6);
		std::cout << "Client[" << i + 1 << "] Info: " << std::endl;
		std::cout << "Position: (" << playerinfo[i].cameraEYE.x << ", "
			<< playerinfo[i].cameraEYE.y << ", " << playerinfo[i].cameraEYE.z << ")" << std::endl;
		std::cout << "Angle: (" << playerinfo[i].Angle.x << ", "
			<< playerinfo[i].Angle.y << ")" << std::endl;
		std::cout << "첫번쨰 좀비의 좌표 x:" << enemyList[0]->getLoc().x <<" y:" << enemyList[0]->getLoc().y << " z:" << enemyList[0]->getLoc().z << endl;
	}
	LeaveCriticalSection(&consoleCS);*/
	}
}




// 이건 아직 미완이죠?
// 인자에 다른 클라이언트의 playerinfo를 받을 수 있게 구조체를 해야됨. 
DWORD WINAPI TCPServer::ClientThread(LPVOID arg) {
	ThreadArg* data = static_cast<ThreadArg*>(arg);
	SOCKET clientSocket = data->clientSocket;
	int clientIndex = data->clientIndex;
	TCPServer* server = data->serverInstance; // Access the TCPServer instance



	int recvSize;


	// Main thread loop
	while (true) {
		//ResetEvent(server->m_hUpdateEvent[clientIndex]);
		// Receive PlayerInfo structure from client
		recvSize = recv(clientSocket, (char*)&server->updateInfo[clientIndex], sizeof(server->updateInfo[clientIndex]), MSG_WAITALL);
		if (recvSize <= 0) {
			std::cout << "Client " << clientIndex << " disconnected or error occurred. Closing connection." << std::endl;
			/*EnterCriticalSection(&server->consoleCS);
			SetCursorPosition(0, 5);
			LeaveCriticalSection(&server->consoleCS);*/
			break;
		}

		server->players[clientIndex]->setLoc(server->updateInfo[clientIndex].cameraEYE);		//임시 코드 updateinfo로 하면 지울거

		// 1이 공격 0이 이동
		if (server->updateInfo[clientIndex].flag == 1) {
			int weaponNumber = server->updateInfo[clientIndex].weaponType;
			server->players[clientIndex]->setweapon(weaponNumber);
			WaitForSingleObject(server->hWriteEvent, INFINITE);
			server->players[clientIndex]->attack_check(server->enemyList, &server->updateInfo[clientIndex], weaponNumber);
			// 여기서 이벤트 사용 하나 더 할 예정
			// std::cout << "Player " << clientIndex << " is attacking!" << std::endl;
			// 여기에 좀비 체력 업데이트 함수 사용
			//SetCursorPosition(0, 20);
			//std::cout << "클라이언트" << clientIndex + 1 << "가 공격" << std::endl;
		

			SetEvent(server->hWriteEvent);
			continue;
		}

		//SetEvent(server->client_events[clientIndex]);

		WaitForSingleObject(server->m_hUpdateEvent, INFINITE);

	//// Prepare renderInfo to send back to the client
		server->FillRenderInfo(server->renderInfo[clientIndex], server->enemyList, server->players[clientIndex]);

		// Send renderInfo back to the client
		send(clientSocket, (char*)&server->renderInfo[clientIndex], sizeof(server->renderInfo[clientIndex]), 0);
	}


	delete data; // Free the dynamically allocated ThreadArg
	closesocket(clientSocket);
	return 0;
}

void TCPServer::FillRenderInfo(RenderInfo& renderInfo, const std::vector<EnemyBase*>& enemyList, Player* player)
{
	renderInfo.HP = player->getHP();
	renderInfo.ammo = player->getammo();

	for (size_t i = 0; i < enemyList.size(); ++i) {
		renderInfo.alive_enemy[i].HP = enemyList[i]->getHP();
		renderInfo.alive_enemy[i].Pos = enemyList[i]->getLoc();
		renderInfo.alive_enemy[i].Rot = enemyList[i]->getRot();
	}

	//renderInfo.alive_num = enemyList.size(); // Total number of enemies
	//renderInfo.remainTime = player->getRemainingTime();
}



