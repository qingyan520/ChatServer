#include"User.hpp"

class GroupUser:public User
{
  public:
    void SetRole(string role)
    {
      this->role=role;
    }
    
    string GetRole()
    {
      return this->role;
    }
  
  private:
    string role;
};
