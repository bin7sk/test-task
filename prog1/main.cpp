#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <algorithm>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <unistd.h>

std::mutex mutex;
std::condition_variable cv;

//shared memory
std::string buffer;
bool new_data = false;

#define PORT 8080
#define SERVER_PATH "/tmp/server"

bool is_only_digits(const std::string &s);

void thread2(){
	while(true){
		std::string s;
		{
			std::unique_lock<std::mutex> lk(mutex);
			cv.wait(lk, []{return new_data;});	

			s = buffer;
			buffer.clear();
			new_data = false;

			lk.unlock();	
		}
		cv.notify_one();
		std::cout<<s<<std::endl;
		int sum = 0;
		for(size_t i=0; i<s.size();i++){
			if(s[i]>='0' && s[i]<='9'){
				sum += s[i] - '0';
			}
		}

		int sock = socket(AF_UNIX, SOCK_STREAM, 0);
		if(sock < 0){
			perror("socket() failed");
			continue;
		}

		struct sockaddr_un serv_addr;
		memset(&serv_addr, 0, sizeof(serv_addr));
		serv_addr.sun_family = AF_UNIX;
		strcpy(serv_addr.sun_path, SERVER_PATH);

		int con = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
		if(con < 0){
			perror("connect() failed");
			close(sock);
			continue;
		}

		int wr = write(sock, &sum, sizeof(int));
		if(wr != sizeof(int)){
			std::cerr<<"Error while writing\n";
		}
		close(sock);
	}
}

int main(void){
	std::thread t2(thread2);

	while(true){
		std::string s;
		std::cin>>s;
		if(s.size() > 64 || !is_only_digits(s))
			continue;
		sort(s.begin(), s.end(), std::greater<char>());
		for(size_t i=0;i<s.size();i++){
			if((s[i] - '0')%2 == 0){
				s.replace(i, 1, "KB");
				i++;
			}
		}
		
		{
			std::unique_lock<std::mutex> lk(mutex);
			cv.wait(lk, []{return !new_data;});	

			buffer = s;
			new_data = true;

			lk.unlock();	
		}
		cv.notify_one();
	}

	t2.join();
	return 0;

}

bool is_only_digits(const std::string &s){
	for(size_t i=0; i<s.size();i++){
		if(!(s[i]>='0' && s[i]<='9'))
			return false;
	}
	return true;
}
