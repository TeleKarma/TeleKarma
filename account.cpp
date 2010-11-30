/*
 * account.cpp
 *
 * Classes for the management of account data.
 *
 */

#include <ptlib.h>
#include <ptlib\textfile.h>
#include <ptlib\filepath.h>
#include <ptlib\channel.h>
#include "account.h"
//#include <stdio.h>
//#include <iostream>
//using namespace std;
/**
 * Construct an Account with a default STUN server.
 */
Account::Account(const PString & name, const PString & registrar, const PString & user)
	: name(name), registrar(registrar), user(user), stun(DEFAULT_STUN_SERVER), index(-1)
{ }

/**
 * Construct an Account with user-specified STUN server.
 */
Account::Account(const PString & name, const PString & registrar, const PString & user, const PString & stun)
	: name(name), registrar(registrar), user(user), stun(stun), index(-1)
{ }

/**
 * Destructor. Does nothing.
 */
Account::~Account() { }

/**
 * Returns the STUN server.
 */
const PString & Account::GetStunServer() const
{
	return stun;
}

/**
 * Returns the registrar.
 */
const PString & Account::GetRegistrar() const
{
	return registrar;
}

/**
 * Returns the user (account) name.
 */
const PString & Account::GetUser() const
{
	return user;
}

/**
 * Returns the account name.
 */
const PString &Account::GetName() const
{
	return name;
}

/**
 * Sets the STUN server.
 */
void Account::SetStunServer(const PString & value)
{
	stun = value;
}

/**
 * Sets the registrar.
 */
void Account::SetRegistrar(const PString & value)
{
	registrar = value;
}

/**
 * Sets the user (account) name.
 */
void Account::SetUser(const PString & value)
{
	user = value;
}

/**
 * Sets the account name.
 */
void Account::setName(const PString & value)
{
	name = value;
}

/**
 * Sets the index of this Account in an AccountList.
 * For use by AccountList only.
 */
void Account::SetIndex(int i)
{
	index = i;
}

/**
 * Returns the index of this Account in the AccountList
 * or -1 if not managed by an AccountList.
 */
int Account::GetIndex() const
{
	return index;
}

/**
 * Loads a new AccountList from a file. If the file
 * does not exist, the AccountList will be empty.
 */
AccountList::AccountList(const PString & filename)
	: fname(filename), list(NULL), size(0)
{
	int totalaccounts = 0;
	int accountnum = 0;
	PString name, registrar, username = "";
	PString stun = DEFAULT_STUN_SERVER;
	PString line = "";
	file = new PTextFile(filename, PFile::ReadOnly);

	if(file->IsOpen()) {
		while(file->ReadLine(line)){ //count # of accounts in file
			if(line[0] == '>') {
				totalaccounts++;
			}
		}
		if(totalaccounts == 0) {
			totalaccounts = 1;
		}

		list = new Account *[totalaccounts]; //allocate for num of account		
		file->SetPosition(0);
		while(file->ReadLine(line)){
			if(line[0] == '>') { //found an account
				line.Delete(0,1);
				if(name != "") {
					list[accountnum++] = new Account(name, registrar, username, stun);
					list[accountnum-1]->SetIndex(accountnum-1);
					stun = DEFAULT_STUN_SERVER;
				}
				name = line;
			}
			else { //not an account line
				//split on ':'
				PStringArray values = line.Tokenise(":");
				if(values[0] == "stun") {
					stun = values[1];
				}
				else if(values[0] == "registrar") { 
					registrar = values[1];
				}
				else if(values[0] == "username") {
					username = values[1];
				}				
			}
		}
		list[accountnum] = new Account(name, registrar, username, stun);
		list[accountnum]->SetIndex(accountnum);
	}
	else { //if the file wasn't found or couldn't be opened
		//printf("file couldn't open");
		name = ""; //these may be redundant
		stun = DEFAULT_STUN_SERVER;
		registrar = "";
		username = "";
		totalaccounts = 1;

		list = new Account *[totalaccounts]; //allocate for num of account
		list[0] = new Account(name, registrar, username, stun);
		list[0]->SetIndex(0);
	}
	size = totalaccounts;
	file ->Close();
}

/**
 * Deallocates all internal memory plus all Account
 * objects. Note that deleting this object will render
 * pointers returned by GetAccount() unusable, since
 * that heap memory will be deallocated.
 */
AccountList::~AccountList()
{
	for (int i = 0; i < size; ++i) {
		delete list[i];
		list[i] = NULL;
	}
	delete[] list;
}

/**
 * Returns a pointer to an account in the list of
 * accounts, or NULL if no such account exists.
 * This class retains responsibility for memory management.
 * Deleting the returned pointer is an error. However,
 * modification of the Account is allowed and expected as
 * the means of editing contents of the list in place.
 * @param index optional; the array index of the account 
 *              sought; zero is the default index.
 * @return requested account or NULL if the index is
 *         out of range.
 */
Account * AccountList::GetAccount(int index)
{
	if (index < 0 || index >= size) return NULL;
	return list[index];
}

/**
 * Add a new account to this list. Note that it is
 * an error to add the same Account (the same heap
 * memory space) to multiple AccountList objects.
 * This AccountList will assume responsibility for
 * managing the memory associated with the passed
 * Account. It is an error for the caller to 
 * subsequently delete the heap memory allocated
 * to the pointer passed into this method.
 */
void AccountList::AddAccount(Account * account)
{
	Account ** n = new Account*[size+1];
	for (int i = 0; i < size; ++i) {
		n[i] = list[i];
	}
	n[size] = account;
	account->SetIndex(size);
	++size;
	delete[] list;
	list = n;
}

/**
 * Remove an account from the list. Note that this
 * Account's heap memory will be deallocated, and 
 * pointers to this account retrieved using 
 * GetAccount() and the pointer passed in via 
 * AddAccount(Account *) (when applicable) will be 
 * left dangling.
 * Also note that the parameter must appear in this
 * list and must be managed only by this AccountList.
 */
void AccountList::RemoveAccount(const Account * account)
{
	if (account == NULL) return;
	int idx = account->GetIndex();
// begin debug
//	for (int i = 0; i < size; ++i) {
//		cout << "'" << list[i]->GetStunServer() << "' " << list[i]->GetIndex() << endl << flush;
//	}
	if (size == 0) return;
	if (idx < 0 || idx >= size) return;
	Account ** n = new Account*[size-1];
	for (int i = 0; i < size; ++i) {
		if (i < idx) {
			n[i] = list[i];
		} else if (i > idx) {
			n[i-1] = list[i];
			if (n[i] != NULL) {
				n[i]->SetIndex(i);
			}
		}
	}
	--size;
	delete account;
	delete[] list;
	list = n;
	// begin debug
//	for (int i = 0; i < size; ++i) {
//		cout << "'" << list[i]->GetStunServer() << "' " << list[i]->GetIndex() << endl << flush;
//	}
}

/**
 * Moves an account to the first array index.
 * The parameter must exist in the list.
 */
void AccountList::SetDefault(const Account * account)
{
	int idx = account->GetIndex();
	if (idx <= 0 || idx >= size) return;
	Account * a;
	for (int i = idx; i > 0; --i) {
		a = list[i];
		list[i] = list[i-1];
		list[i-1] = a;
	}
}

/**
 * Returns the number of Accounts in the list.
 */
int AccountList::GetCount() const
{
	return size;
}

/**
 * Saves the AccountList to the file provided in
 * the constructor. If that file does not exist, it
 * will be created if possible.
 * @return if successful
 */
bool AccountList::Save() const
{
	//printf("in save\n");
	return SaveTo(fname);
}

/**
 * Saves the AccountList to a file whose name is provided
 * by the caller. If that file does not exist, it
 * will be created if possible. Use to make a backup
 * copy of the account config file.
 * @return if successful
 */
bool AccountList::SaveTo(const PString & filename) const
{
	//printf("in SaveTo()\n");
	file->Open(filename, PFile::WriteOnly);
	if(file->IsOpen()) { //if the file already exists
		//printf("File opened %i", size);
		for(int i=0;i<size;i++) {
			//printf("Writing first line...");
			file->WriteLine(">"+list[i]->GetName());
			//printf("Writing second line...");
			file->WriteLine("stun:"+list[i]->GetStunServer());
			file->WriteLine("registrar:"+list[i]->GetRegistrar());
			file->WriteLine("username:"+list[i]->GetUser());
			//printf("Finished writing all \n");
		}
	}

	return false;
}
