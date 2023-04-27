#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <csignal>
#include <ctime>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

bool isStatisticEnabled;
bool flag;

class Process {
private:
    pid_t pid;
public:
    Process(pid_t pid) : pid(pid) {}

    pid_t get_pid() { return pid; }
};

vector<Process *> processes;

struct stat {
    int n1;
    int n2;
} stat;

struct pairStat {
    int n00;
    int n01;
    int n10;
    int n11;
} pair_stat;

void signal_handler(int signum) {
    if (signum == SIGUSR1)
        isStatisticEnabled = true;

    if (signum == SIGUSR2)
        isStatisticEnabled = false;

    if (signum == SIGALRM) {
        if (stat.n1 == 0 && stat.n2 == 0)
            pair_stat.n00++;
        if (stat.n1 == 0 && stat.n2 == 1)
            pair_stat.n01++;
        if (stat.n1 == 1 && stat.n2 == 0)
            pair_stat.n10++;
        if (stat.n1 == 1 && stat.n2 == 1)
            pair_stat.n11++;
        flag = true;
    }

    if (signum == SIGINFO)
    {
        string info;
        info = "PID: " + to_string(getpid()) + " PPID: " + to_string(getppid()) + " " +
               to_string(pair_stat.n00) + " " + to_string(pair_stat.n01) + " " + to_string(pair_stat.n10) +
               " " + to_string(pair_stat.n11) + "\n";

        for (int i = 0; i < static_cast<int>(info.size()); ++i) {
            fputc(info[i], stdout);
        }
        fflush(stdout);
    }
}

void print_stat() {
    struct timespec t = {0, 50000000};

    int counter = 0;

    while (1) {
        nanosleep(&t, NULL);
        kill(getpid(), SIGALRM);

        while (1) {

            if (stat.n1 == 0 && stat.n2 == 0)
                stat.n1 = stat.n2 = 1;
            else
                stat.n1 = stat.n2 = 0;

            if (flag) {
                flag = false;
                break;
            }
        }

        if (counter == 101) {
            if (isStatisticEnabled)
            {
                string info;
                info = "PID: " + to_string(getpid()) + " PPID: " + to_string(getppid()) + " " +
                       to_string(pair_stat.n00) + " " + to_string(pair_stat.n01) + " " + to_string(pair_stat.n10) +
                       " " + to_string(pair_stat.n11) + "\n";

                for (int i = 0; i < static_cast<int>(info.size()); ++i) {
                    fputc(info[i], stdout);
                }
                fflush(stdout);

                counter = 0;
            } else
                counter = 0;
        }
        counter++;
    }
}

void create_process()
{
    pid_t proc = fork();
    if (proc == -1)
        cout << "Create error" << endl;
    if (proc == 0)
    {
        isStatisticEnabled = true;
        print_stat();
    }
    if (proc > 0)
    {
        Process *newProc = new Process(proc);
        processes.emplace_back(newProc);
        cout << "Process created successfully" << endl;
    }
}

void delete_last_process()
{
    if (processes.empty())
    {
        cout << "No processes" << endl;
        return;
    }

    pid_t pid = processes[processes.size() - 1]->get_pid();

    if (kill(pid, SIGTERM) == 0)
    {
        int status;
        waitpid(pid, &status, 0);
        delete processes[processes.size() - 1];
        processes.pop_back();
        cout << "Process deleted successfully. Remaining " << processes.size() << " processes" << endl;
    }
    else
        cout << "Error during deletion" << endl;
}

void print_all_processes()
{
    cout << "Parent process:\n" << '\t' << getpid() << endl;
    cout << "Child processes: " << endl;
    if (!processes.empty())
    {
        for (int i = 0; i < static_cast<int>(processes.size()); i++)
            cout << '\t' << i + 1 << ' ' << processes[i]->get_pid() << endl;
    }
}

void delete_all_processes()
{
    if (processes.empty())
    {
        cout << "No processes" << endl;
        return;
    }

    for (int i = 0; i < static_cast<int>(processes.size()); i++)
    {
        if (kill(processes[i]->get_pid(), SIGTERM) == 0)
        {
            int status;
            waitpid(processes[i]->get_pid(), &status, 0);
            delete processes[i];
        } else
            cout << "Error during deletion" << endl;
    }
    processes.clear();

    cout << "All processes deleted" << endl;
}

int main() {
    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, signal_handler);
    signal(SIGALRM, signal_handler);
    signal(SIGINFO, signal_handler);

    cout << "+: создать дочерний процесс\n"
            "-: удалить последний созданный процесс\n"
            "l: вывести все родительские и дочерние процессы\n"
            "k: удалить все дочерние процессы\n"
            "s: запретить вывод статистики\n"
            "g: разрешить вывод статистики\n"
            "s<num>: запретить вывод статистики num-процессу\n"
            "g<num>: разрешить вывод статистики num-процессу\n"
            "p<num>: запрет вывода для всех, num-процесс выводит статистику"
            "q: завершить программу\n"<< endl;

    while (1) {
        string ch;
        rewind(stdin);
        cin >> ch;
        rewind(stdin);

        if (ch == "q")
        {
            delete_all_processes();
            exit(0);
        }
        else if (ch == "+")
            create_process();

        else if (ch == "-")
            delete_last_process();

        else if (ch == "l")
            print_all_processes();

        else if (ch == "k")
            delete_all_processes();

        else if (ch == "s")
        {
            if (processes.empty())
            {
                cout << "No processes" << endl;
                continue;
            }
            for (int i = 0; i < static_cast<int>(processes.size()); i++)
            {
                if (kill(processes[i]->get_pid(), SIGUSR2) != 0)
                {
                    cout << "Stopping error" << endl;
                    break;
                }
            }
            cout << "Processes stopped successfully" << endl;

        } else if (ch == "g")
        {
            if (processes.empty())
            {
                cout << "No processes" << endl;
                continue;
            }
            for (int i = 0; i < static_cast<int>(processes.size()); i++)
            {
                if (kill(processes[i]->get_pid(), SIGUSR1) != 0) {
                    cout << "Enebling error" << endl;
                    break;
                }
            }
            cout << "Processes enabled successfully" << endl;
        }
        else
        {
            if (ch[0] == 's')
            {
                if (processes.empty())
                {
                    cout << "No processes" << endl;
                    continue;
                }
                int num = stoi(ch.substr(1));
                if (kill(processes[num]->get_pid(), SIGUSR2) == 0)
                    cout << "Process " << num << " stopped successfully" << endl;
                else
                    cout << "Stopping error" << endl;
            }
            if (ch[0] == 'g')
            {
                if (processes.empty())
                {
                    cout << "No processes" << endl;
                    continue;
                }
                int num = stoi(ch.substr(1));
                if (kill(processes[num]->get_pid(), SIGUSR1) == 0)
                    cout << "Process " << num << " enabled successfully" << endl;
                else
                    cout << "Enabling error" << endl;
            } else if (ch[0] == 'p')
            {
                if (processes.empty())
                {
                    cout << "No processes" << endl;
                    continue;
                }

                int num = stoi(ch.substr(1));

                for (int i = 0; i < static_cast<int>(processes.size()); i++)
                {
                    if (kill(processes[i]->get_pid(), SIGUSR2) != 0)
                        cout << "Stopping error" << endl;
                }

                kill(processes[num]->get_pid(), SIGINFO);

                fd_set timer;
                FD_ZERO(&timer);
                FD_SET(STDIN_FILENO, &timer);

                struct timeval timeout;
                timeout.tv_sec = 20;
                timeout.tv_usec = 0;

                int selectResult = select(STDIN_FILENO + 1, &timer, NULL, NULL, &timeout);

                if (selectResult == -1)
                    std::cerr << "Ошибка таймера" << std::endl;
                else if (selectResult == 0)
                {
                    std::cout << "End of timer" << std::endl;
                    for (int i = 0; i < static_cast<int>(processes.size()); i++)
                    {
                        if (kill(processes[i]->get_pid(), SIGUSR1) != 0)
                            cout << "Enabling error" << endl;
                    }
                }
                else
                {
                    if (FD_ISSET(STDIN_FILENO, &timer))
                    {
                        std::string input;
                        std::cin >> input;

                        if (input == "g")
                        {
                            for (int i = 0; i < static_cast<int>(processes.size()); i++)
                            {
                                if (kill(processes[i]->get_pid(), SIGUSR1) != 0)
                                    cout << "Enabling error" << endl;
                            }
                            continue;
                        }
                    }
                }
            }
        }
    }
}