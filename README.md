# ERack
Event tracking app, you give an event and ppl give their time

General Idea:
1. User creates an event and sets available times.
2. User Gets link for event.
3. User shares link with other users.
4. Other users give their available times.
5. App shows everyone's times and the best time (time with most overlap).

Requirements:
- Some frontend (mobile, desktop, and/or web application, it'll handle data processing (aka it'll find the most suitable time)).
- Some backend (just to smooth data transfer and sync stored data).
- Some way to store data such that 2 ppl can send results without conflict SQL? JSON? text files?.

Data:
- Available time list: a list of start and end times
- Event: Contains available time list and user list
- User : Created per event and contains available times list

## Compiling Crow Backend
Run the following command in `/backend/cpp`, you'll need `boost` installed (easy on linux, might as well give up on windows).
```
g++ main.cpp -lpthread -o erack_be
```
