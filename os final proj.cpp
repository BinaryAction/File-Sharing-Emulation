// os final proj.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>

class folder;
class profile;

std::vector<std::string> splitString(std::string s, std::string seperator, int offset) {
    // seperate by space
    int stringcount = 0;
    std::string segment = "";
    std::vector<std::string> strings;
    for (int i = offset; i < s.length(); i++) { // offset=1 to ignore the /
        if (s.substr(i, 1) == seperator) {
            stringcount++;
            strings.push_back(segment);
            segment = "";
        }
        else {
            segment += s.substr(i, 1);
        }
    }
    // so it can push back the last segment
    strings.push_back(segment);
    return strings;
}

// Admins can edit anyone files and delete regular users profiles

class folder {
    std::string name; // nam of folder
    std::map<std::string, std::string> contents; // actual files and their contents
    std::vector<profile> collaborators; // file sharing
};

class profile {
public:
    std::string username;
    std::string password;
    int permissionLevel = 0; // user=0, admin=1, default=user
    static void AttemptLogIn(profile p);
    void logout();
    profile getProfileByUsername(); // pls implement this asap
    std::vector<std::string> collaborators;
    void invite(std::string username);
    bool locked = false;
};

// Each profile can invite another profile to work with them

// as [profile] sendinvite [profile]
// as [profile] cancelinvite [profile]
// as [profile] viewinvites
// as [profile] sharedwith
// as [profile] viewoutbound invites
// as [profile] acceptcollab [profile]
// as [profile] rejectcollab [profile]


// as editfile [owner] [filename]
// as viewfile [owner] [filename]
// as deletefile [owner] [filename]
// as [profile] createfile [filename]

std::vector<profile> usersLoggedIn;
std::vector<std::string[2]> invitations;
// map(string, map(string, string))
std::map<std::string, std::map<std::string, std::string>> database; // string->username, map->userdata
std::map<profile, std::vector<std::string>> invites;
std::map<std::string, profile> usernameToProfileMap;
std::vector<std::string> commandList;

bool updateDatabase() {
    // overwrite database
    std::ofstream outFile;
    outFile.open("users.txt");
    if (outFile.fail()) {
        std::cout << "problem updating database...";
        return false;
    }
    if (outFile.is_open()) {
        std::map<std::string, std::map<std::string, std::string>>::iterator it = database.begin();
        while (it != database.end()) {
            std::string collabnames = "";
            std::vector<std::string> collabnamesvector = splitString(it->second["collaborators"], "-", 0);
            for (auto x : collabnamesvector) {
                collabnames += x;
                if (x != collabnamesvector[collabnamesvector.size()-1]) {
                    collabnames += "-";
                }
            }
            //std::cout << collabnames << "\n";
            outFile << it->first << ":" << it->second["password"] << ":" << it->second["PermissionLevel"] << ":" << collabnames << ":" << it->second["lockstatus"] << "\n";
            it++;
        }
        outFile.close();
    }
    return true;
}

void profile::invite(std::string username) {
    for (auto x : collaborators) {
        if (x == username) {
            std::cout << username << " is already granted permission.\n";
            return;
        }
    }

    collaborators.push_back(username);
    std::string collabhyphened = "";
    for (std::string x : collaborators) {
        if (x != collaborators[collaborators.size() - 1]) {
            collabhyphened += x + "-";
        }
        else {
            collabhyphened += x;
        }
    }
    database[this->username]["collaborators"] = collabhyphened;
    std::cout << "Successfully added " << username << " to work on your files!\n";
    updateDatabase();
}

void profile::logout() {
    auto it = usersLoggedIn.begin();
    while (it != usersLoggedIn.end()) {
        if (this->username == it->username) {
            std::cout << "Successfully logged out of user " << it->username << "\n";
            usersLoggedIn.erase(it);
            return;
        }
        it++;
    }
    std::cout << "Error logging out: user was never logged in.\n";
}

void profile::AttemptLogIn(profile p) {
    if (database.find(p.username) != database.end() && database[p.username]["password"] == p.password) {
        std::cout << "Successfully logged in as " << p.username << "!\n";
        // assumes permission level exists
        p.permissionLevel = std::stoi(database[p.username]["PermissionLevel"]);
        usersLoggedIn.push_back(p);
        // check if we have data for their file


    }
    else {
        std::cout << "Invalid login for " << p.username << "!\n";
    }
}

void execute(std::string command) {

    if (command.substr(0, 1) == "/") {
        // break the command up by spaces
        std::vector<std::string> data = splitString(command, " ", 1);
        // login [username] [password]
        if (data[0] == "login" && data.size() == 3) {
            profile p;
            p.username = data[1];
            p.password = data[2];
            //p.permissionLevel = std::stoi(database[p.username]["PermissionLevel"]);

            profile::AttemptLogIn(p);
        }
        // register [username] [password] [repeat password]
        else if (data[0] == "register" && data.size() == 4) {
            if (database.count(data[1]) == 0) {
                if (data[2] != data[3]) {
                    std::cout << "Passwords do not match!\n";
                    return;
                }
                profile p;
                p.username = data[1];
                p.password = data[2];
                p.permissionLevel = 0; // default permission level
                p.locked = "unlocked";
                std::map<std::string, std::string> vec;
                vec["password"] = data[2];
                vec["PermissionLevel"] = "0";
                vec["collaborators"] = "self";
                vec["lockstatus"] = "unlocked";
                database[data[1]] = vec;
                std::cout << "Successfully registered user with name " << data[1] << "\n";
                updateDatabase();
                profile::AttemptLogIn(p);
            }
            else {
                std::cout << "That username is already taken!\n";
            }  
        }
        // loggedusers
        else if (data[0] == "loggedusers" && data.size() == 1) {
            for (profile p : usersLoggedIn) {
                std::cout << p.username << " ";
            }
            std::cout << "\n";
        }
        // as [profile] [command] [args]
        else if (data[0] == "as" && data.size() >= 3) {
            bool canDoThis = false;
            profile* personusing = nullptr;
            for (auto x : usersLoggedIn) {
                if (x.username == data[1]) {
                    canDoThis = true;
                    personusing = &x;
                    break;
                }
            }
            if (canDoThis) {
                // user specific commands go here
                if (data[2] == "print") {
                    // gotta capture all words!
                    std::string message;
                    for (int i = 3; i < data.size(); i++) {
                        message += data[i] + " ";
                    }
                    std::cout << "[" << data[1] << "] " << message << "\n";
                }
                // as [profile] sharewith [profile]
                else if (data[2] == "sharewith" && data.size() == 4) {
                    personusing->username = data[1];
                    personusing->collaborators = splitString(database[data[1]]["collaborators"], "-", 0);
                    personusing->invite(data[3]);
                }
                // as [profile] sharedwith
                else if (data[2] == "sharedwith" && data.size() == 3) {
                    personusing->username = data[1];
                    personusing->collaborators = splitString(database[data[1]]["collaborators"], "-", 0);
                    std::cout << "Showing list of people who can currently work with your files:\n";
                    for (auto x : personusing->collaborators) {
                        std::cout << x << "\n";
                    }
                    std::cout << "End of Collaborators List !\n";
                }
                // as [profile] promote [profile]
                // promotion of permission level
                else if (data[2] == "promote") {
                    
                }
                else {
                    std::cout << "Invalid command/arguments!\n";
                }
            }
            else {
                std::cout << "Access denied : user not logged in!\n";
            }
        }
        // help
        else if (data[0] == "help" && data.size() == 1) {
            std::cout << "\n! Displaying a list of commands !\n";
            for (int i = 0; i < commandList.size(); i++) {
                std::cout << commandList[i] << "\n";
            }
            std::cout << "\n";
        }
        // logout [username]
        else if (data[0] == "logout" && data.size() > 1) {
            // check if user is even logged in to begin with
            std::vector<profile>::iterator it = usersLoggedIn.begin();
            while (it != usersLoggedIn.end()) {
                if (it->username == data[1]) {
                    // found, log them out
                    it->logout();
                    return;
                }
                it++;
            }
            std::cout << "That user is not logged in!\n";
        }
        else {
            std::cout << "Invalid command/arguments. Please double check!\n";
        }
    }
}

int main()
{
    // import data from database
    std::cout << "Connecting to database...\n";
    std::ifstream inFile;
    inFile.open("users.txt");
    if (inFile.fail()) {
        std::cout << "Failed to open file users.txt. :/\n";
        if (inFile.is_open()) {
            inFile.close();
        }
        return 0;
    }

    std::string line;
    while (inFile >> line) {
        std::vector<std::string> userdata = splitString(line, ":", 0);

        std::map<std::string, std::string> vec;
        vec["password"] = userdata[1];
        vec["PermissionLevel"] = userdata[2];
        vec["collaborators"] = userdata[3];
        vec["lockstatus"] = userdata[4];
        database[userdata[0]] = vec;

        //database[userdata[0]] = userdata[1];
    }

    // close the file after done using it
    if (inFile.is_open()) {
        inFile.close();
    }
    std::cout << "Successfully loaded user database.\n";
    // end of external file

    // read from external folder
    // File name , Path
    std::vector<std::map<std::string, std::string>> workspace;

    commandList.push_back("The prefix for commands is /");
    commandList.push_back("help");
    commandList.push_back("login [username] [password]");
    commandList.push_back("register [username] [password] [repeat password]");
    commandList.push_back("logout [username]");
    commandList.push_back("loggedusers");
    commandList.push_back("as [username] [command] [args]");
    commandList.push_back("as [username] print [string]");
    commandList.push_back("as [username] sharewith [username]");
    commandList.push_back("as [username] sharedwith");
    std::cout << "Type /help for a list of commands\n";
    std::cout << "Type /exit to exit and SAVE.\n";
    std::string input;
    while (true) {
        //std::cin >> input;
        std::getline(std::cin, input);
        if (input != "/exit") {
            execute(input);
        }
        else {
            std::cout << "Saving the database...\n";
            for (auto member : database) {
                member.second["lockstatus"] = "unlocked";
            }
            updateDatabase();
            std::cout << "You may now close the program.\n";
        }

    }
    return 0;
}
