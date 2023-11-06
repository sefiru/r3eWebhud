# WebHUD for R3E
## Portable, small, and fast (no installations required)
- No-delay stating lights.
- ~100fps delta and radar.
- Radar is car size accurate.
### How to Use
- Download the .exe file.
- Add “-webdev -webHudUrl=http://localhost:8081/” to the run parameters in Steam (just like OtterHUD).
- Run the .exe file and the game (in any order).
- During a race, configure the positions of elements by dragging them with the left mouse button. You can also resize them by dragging the blue square with both the left and right mouse buttons. To save the positions, click on the top right corner of the screen. To change positions again, click on the top right corner.

***The program also requires ports 8081 and 8082 to be free for the HTTP and WebSocket servers.*** 

After running the .exe file, a .db file will be created in the same folder. This is a SQLite database file that stores the positions of elements on the screen as well as the best lap/fuel data. You can easily inspect it with any SQL editor.

If you have any questions, you can ask on a "app” channel on the official R3E Discord channel, on the [Forum](https://forum.kw-studios.com/index.php?threads/another-custom-webhud.19310/), or by messaging me directly on Discord at “sefianti”.

*To compile add sqlite amalgamation sources.*
