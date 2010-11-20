/*
 * account.h
 *
 * Classes for the management of account data.
 *
 */

#ifndef _ACCOUNT_H_
#define _ACCOUNT_H_

#define DEFAULT_STUN_SERVER "stun.ekiga.net"

class PString;
class AccountList;

/**
 * Represents a user's account with a SIP service provider.
 */
class Account
{
	public:

		/**
		 * Construct an Account with a default STUN server.
		 */
		Account(const PString & registrar, const PString & user);

		/**
		 * Construct an Account with user-specified STUN server.
		 */
		Account(const PString & registrar, const PString & user, const PString & stun);

		/**
		 * Destructor. Does nothing.
		 */
		virtual ~Account();

		/**
		 * Returns the STUN server.
		 */
		const PString & GetStunServer() const;

		/**
		 * Returns the registrar.
		 */
		const PString & GetRegistrar() const;

		/**
		 * Returns the user (account) name.
		 */
		const PString & GetUser() const;

		/**
		 * Sets the STUN server.
		 */
		void SetStunServer(const PString & value);

		/**
		 * Sets the registrar.
		 */
		void SetRegistrar(const PString & value);

		/**
		 * Sets the user (account) name.
		 */
		void SetUser(const PString & value);

	private:

		PString registrar;
		PString user;
		PString stun;
		int index;
		
		/**
		 * Sets the index of this Account in an AccountList.
		 * For use by AccountList only.
		 */
		void SetIndex(int i);
		
		/**
		 * Returns the index of this Account in the AccountList
		 * or -1 if not managed by an AccountList.
		 */
		int GetIndex() const;
	
	friend class AccountList;
};

/**
 * Represents a persistant list of SIP service provider accounts.
 */
class AccountList
{
	public:
		
		/**
		 * Loads a new AccountList from a file. If the file
		 * does not exist, the AccountList will be empty.
		 */
		AccountList(const PString & filename);
		
		/**
		 * Deallocates all internal memory plus all Account
		 * objects. Note that deleting this object will render
		 * pointers returned by GetAccount() unusable, since
		 * that heap memory will be deallocated.
		 */
		virtual ~AccountList();
		
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
		Account * GetAccount(int index = 0);
		
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
		void AddAccount(Account * account);
		
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
		void RemoveAccount(const Account * account);
		
		/**
		 * Moves an account to the first array index.
		 * The parameter must exist in the list.
		 */
		void SetDefault(const Account * account);
		
		/**
		 * Returns the number of Accounts in the list.
		 */
		int GetCount() const;
		
		/**
		 * Saves the AccountList to the file provided in
		 * the constructor. If that file does not exist, it
		 * will be created if possible.
		 * @return if successful
		 */
		bool Save() const;
		
		/**
		 * Saves the AccountList to a file whose name is provided
		 * by the caller. If that file does not exist, it
		 * will be created if possible. Use to make a backup
		 * copy of the account config file.
		 * @return if successful
		 */
		bool SaveTo(const PString & filename) const;

	private:
		PString fname;
		Account ** list;
		int size;
};

#endif // _ACCOUNT_H_