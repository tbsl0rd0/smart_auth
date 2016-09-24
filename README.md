# smart_auth
> smart_auth is helper solution to manage your local computer's windows logon authentication

## required
> platform : windows 7+ x64

## program characteristic for mechanism
> 1. whenever you execute this program, it will add current user name information (in terms of netBIOS. user-PC for example) to registry (HKLM/SOFTWARE/WinAuth)
> 2. like number 1, it will add some other information (auth stack, settings info ...) to registry
> 3. whenever you turn on/off checkbox on in settings tab, it will copy credential provider (which is in same folder with exe file) to system32 folder
> 4. like number 3, it will add information for attaching credential provider dll to registry
