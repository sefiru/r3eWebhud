# WebHUD for R3E
## Portable, small, and fast (no installations required)
- No-delay stating lights.
- ~100fps delta and radar.
- Radar is car size accurate.
- Input graph.
- ABS/Grip graph.
### How to Use
- Download the .exe file.
- Add “-webdev -webHudUrl=http://localhost:8081/” to the run parameters in Steam (just like OtterHUD).
- Run the .exe file and the game (in any order).
- Now before using u can change settings in settings.txt file and restart .exe.
- During a race, configure the positions of elements by dragging them with the left mouse button. You can also resize them by dragging the element with a right mouse buttons. To save the positions, click on the top right corner of the screen. To change positions again, click on the top right corner.

- To create your own skins, change the ‘dev’ value to 1 in the settings.txt file. Now, you can edit the index.html file and see the changes in the game after pressing ESC and Resume to reload the “page”.

***The program also requires ports 8081 and 8082 to be free for the HTTP and WebSocket servers.(Changeable in settings.txt)*** 

## ABS graph
##### At first you will need create a data for each car that you want to show abs data:
- Go to **Game Settings -> Gameplay -> Assists -> Braking Assists**, and turn off the ABS.
- Create a game (single on daytona oval is a good choice) with car that you want.
- In the car setup set the break bias to 50% and pressure to 100%.
- Now in WebHud console type "record" (or just "r"), you should see "record started" in the console.
- Start driving the car with brake slightly pressed. Heat the brakes until 1500 degrees (or less if you are sure the temperature wont go that high in real race).
- When warming up ends, type in the console "stop" (or just "s").
- You will see information about name of a saved datafile.
- If you see list of temperatures and wheels, its about temperature didn't record, can happend if temperature rises too quickly or just higher values that you skipped by yourself, to make data complete, simply return to the garage, start recording again, and repeat the process.
- Well done! Now turn on the ABS in Game Settings and enjoy the ABS graph (if you are using brake pressure that is not 100%, just type the number in the console).
#### You can also share your recorded data by posting it on the [Forums topic](https://forum.kw-studios.com/index.php?threads/another-custom-webhud.19310/), or by making Git pull request to the repo.

After running the .exe file, a .db file will be created in the same folder. This is a SQLite database file that stores the positions of elements on the screen as well as the best lap/fuel data. You can easily inspect it with any SQL editor.

If you have any questions, you can ask on a "apps” channel on the official R3E Discord channel, on the [Forum](https://forum.kw-studios.com/index.php?threads/another-custom-webhud.19310/), or by messaging me directly on Discord at “sefianti”.

*To compile add sqlite amalgamation sources.*

![Alt text](https://sun9-78.userapi.com/impg/rbzOVdFpqCE6JOTH9gun6ZwRADOhLVelQr-Bug/5VK95oAviJQ.jpg?size=1920x1080&quality=95&sign=2f77848d3cadf4f9fe6b76050acefc4b&type=album "screenshot")
