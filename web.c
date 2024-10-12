#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#include "web.h"
#include "handshakeKey.h"
#include "db.h"
#include "webhudgit.h"

#pragma comment(lib, "ws2_32.lib")

char* response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
char* responz1 = "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n<meta charset=\"UTF-8\">\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n<title>WebHud</title>\n<script>\nfunction float16ToFloat32(uint16) {\nconst sign = (uint16 & 0x8000) >> 15;  // Extract the sign bit (1 bit)\nconst exponent = (uint16 & 0x7C00) >> 10;  // Extract the exponent (5 bits)\nconst mantissa = uint16 & 0x03FF;  // Extract the mantissa (10 bits)\n// Handle special cases\nif (exponent === 0) {\nif (mantissa === 0) {\n// Zero value (either positive or negative)\nreturn sign === 0 ? 0 : -0;\n}\n// Subnormal number\nreturn (sign ? -1 : 1) * Math.pow(2, -14) * (mantissa / 1024);\n} else if (exponent === 31) {\nif (mantissa === 0) {\n// Infinity (positive or negative)\nreturn sign ? -Infinity : Infinity;\n}\n// NaN (Not a Number)\nreturn NaN;\n}\n// Normalized number\nreturn (sign ? -1 : 1) * Math.pow(2, exponent - 15) * (1 + mantissa / 1024);\n}\n// document.addEventListener('keydown', function(event) {\n//     if (event.key === 'Escape' || event.key === 'Esc') {\n//         socket.send(\"kek\");\n//         socket.close();\n//     }\n// });\nwindow.addEventListener('beforeunload', function (event) {\n// Custom logic before the page is closed\n// Example: Closing a WebSocket connection\nsocket.send(\"k\");\nsocket.close();\n});\nDataView.prototype.getUTF16String = function (offset, length) {\nvar utf16 = new ArrayBuffer(length * 2);\nvar utf16View = new Uint16Array(utf16);\nfor (var i = 0; i < length; ++i) {\nutf16View[i] = this.getUint8(offset + i);\n}\nreturn String.fromCharCode.apply(null, utf16View);\n};\nfunction getTemperatureColor(temp, cold_temp, optimal_temp, hot_temp) {\nvar color;\nif (temp <= cold_temp) {\ncolor = `rgb(0,0,255)`;\n} else if (temp >= hot_temp) {\ncolor = `rgb(255,0,0)`;\n} else if (temp == optimal_temp) {\ncolor = `rgb(0,255,0)`;\n} else if (temp < optimal_temp) {\nvar ratio = (temp - cold_temp) / (optimal_temp - cold_temp);\ncolor = `rgb(0, ${Math.floor(255 * ratio)}, ${Math.floor(255 * (1 - ratio))})`;\n} else {\nvar ratio = (temp - optimal_temp) / (hot_temp - optimal_temp);\ncolor = `rgb(${Math.floor(255 * ratio)}, ${Math.floor(255 * (1 - ratio*1.2))}, 0)`;\n}\nreturn color;\n}\nconst socket = new WebSocket('ws://127.0.0.1:8082');\nvar loadedRat = {};\nloadedRat[-1] = {\n\"UserId\" : -1,\n\"Fullname\" : \"bot\",\n\"Rating\" :0,\n\"Reputation\" : 0\n};\nvar flagStatusYellow = 0;\ndocument.addEventListener(\"DOMContentLoaded\", function (event) {\nvar lastLap = document.querySelector(\"#lastlap\");\nvar bestLapLead = document.querySelector(\"#bestlaplead\");\nvar delta = document.querySelector(\"#delta\");\nvar bestLap = document.querySelector(\"#bestlap\");\nvar fuelMonitorTds = document.querySelectorAll(\"#fuelmonitor td\");\nvar inputsTds = document.querySelectorAll(\"#inputs td\");\nvar wheelsTds = document.querySelectorAll(\"#wheels td\");\nvar relativeTrs = document.querySelectorAll(\"#relative tr\");\nvar relativeTds = {};\nvar startLight = document.querySelector(\"#startLight\");\nvar startLightDivs = document.querySelectorAll(\"#startLight div\");\nvar flTemp1 = document.querySelector(\"#flTemp1\");\nvar flWear = document.querySelector(\"#flWear\");\nvar flPressure = document.querySelector(\"#flPressure\");\nvar flDirt = document.querySelector(\"#flDirt\");\nvar flBrake = document.querySelector(\"#flBrake\");\nvar flTemp2 = document.querySelector(\"#flTemp2\");\nvar flTemp3 = document.querySelector(\"#flTemp3\");\nvar frTemp1 = document.querySelector(\"#frTemp1\");\nvar frWear = document.querySelector(\"#frWear\");\nvar frPressure = document.querySelector(\"#frPressure\");\nvar frDirt = document.querySelector(\"#frDirt\");\nvar frBrake = document.querySelector(\"#frBrake\");\nvar frTemp2 = document.querySelector(\"#frTemp2\");\nvar frTemp3 = document.querySelector(\"#frTemp3\");\nvar rlTemp1 = document.querySelector(\"#rlTemp1\");\nvar rlWear = document.querySelector(\"#rlWear\");\nvar rlPressure = document.querySelector(\"#rlPressure\");\nvar rlDirt = document.querySelector(\"#rlDirt\");\nvar rlBrake = document.querySelector(\"#rlBrake\");\nvar rlTemp2 = document.querySelector(\"#rlTemp2\");\nvar rlTemp3 = document.querySelector(\"#rlTemp3\");\nvar rrTemp1 = document.querySelector(\"#rrTemp1\");\nvar rrWear = document.querySelector(\"#rrWear\");\nvar rrPressure = document.querySelector(\"#rrPressure\");\nvar rrDirt = document.querySelector(\"#rrDirt\");\nvar rrBrake = document.querySelector(\"#rrBrake\");\nvar rrTemp2 = document.querySelector(\"#rrTemp2\");\nvar rrTemp3 = document.querySelector(\"#rrTemp3\");\nvar driverBlocks = document.querySelectorAll(\".driverBlock\");\nvar raceLaps = document.querySelector(\"#raceLaps\");\nvar raceTime = document.querySelector(\"#raceTime\");\nvar standingsFlag = document.querySelector(\"#splitFlag\");\nvar lastLapSetts;\nvar bestLapSetts;\nvar bestLapLeadSetts;\nvar deltaSetts;\nvar radarSetts;\nvar allTimeBestTableSetts;\nvar relativeTableSetts;\nvar wheelsSetts;\nvar inputGraphSetts;\nvar absgripGraphSetts;\nvar slipGraphSetts;\nvar inputPercentsSetts;\nvar standingsSetts;\nvar startingLightsSetts;\nvar chv = [];\nlet canvas1 = document.getElementById('chart');\nlet ctx1 = canvas1.getContext('2d');\n// document.querySelector(\"body\").addEventListener(\"focusout\", (event) => {\n//     //event.target.style.background = \"\";\n//     socket.send(\"uf\");\n// });\nlet data = {\nlabels: [],\ndatasets: [\n{ label: 'Input 1', data: [], color: 'red' },\n{ label: 'Input 2', data: [], color: 'green' },\n{ label: 'Input 3', data: [], color: 'gray' },\n{ label: 'Input 4', data: [], color: 'white' },\n]\n};\nfunction drawChart() {\n// clear the canvas\nctx1.clearRect(0, 0, canvas1.width, canvas1.height);\n// draw the datasets\ndata.datasets.forEach((dataset, i) => {\nctx1.beginPath();\nctx1.strokeStyle = dataset.color;\nctx1.lineWidth = 1.5;\ndataset.data.forEach((point, j) => {\nctx1.lineTo(j * 1.33, (canvas1.height - point +2)  > 150?148:canvas1.height - point +2);\n//ctx1.lineTo(j * 1, canvas1.height - point);\n//ctx1.lineTo(j * 10, point);\n});\nctx1.stroke();\n});\n// draw the labels\nctx1.fillStyle = 'white';\nctx1.fillText(data.labels[data.labels.length - 1], canvas1.width - 50, canvas1.height - 10);\n}\nlet canvas2 = document.getElementById('chartSlip');\nlet ctx2 = canvas2.getContext('2d');\nlet data1 = {\nlabels: [],\ndatasets: [\n{ label: 'Input 1', data: [], color: 'white' },\n{ label: 'Input 2', data: [], color: 'red' },\n]\n};\nlet data2 = {\nlabels: [],\ndatasets: [\n{ label: 'Input 1', data: [], color: 'white' },\n]\n};\nfunction drawChartSlip() {\n// clear the canvas\nctx2.clearRect(0, 0, canvas2.width, canvas2.height);\ndata2.datasets.forEach((dataset, i) => {\ndataset.data.forEach((point, j) => {\nctx2.beginPath();\nctx2.lineWidth = 1.5;\nswitch (point) {\ncase 1: ctx2.strokeStyle = 'rgba(255, 0, 0, 0.2)'; break;\ncase 2: ctx2.strokeStyle = 'rgba(0, 255, 0, 0.2)'; break;\ncase 3: return;\n}\nctx2.moveTo(j * 1.33, 2);\nctx2.lineTo(j * 1.33, (canvas2.height - point +2)  > 150?148:canvas2.height - point +2);\n//ctx2.lineTo(j * 1, canvas2.height - point);\n//ctx2.lineTo(j * 10, point);\nctx2.stroke();\n});\n});\n// draw the datasets\ndata1.datasets.forEach((dataset, i) => {\nctx2.beginPath();\nctx2.strokeStyle = dataset.color;\nctx2.lineWidth = 1.5;\ndataset.data.forEach((point, j) => {\nctx2.lineTo(j * 1.33, (canvas2.height - point +2)  > 150?148:canvas2.height - point +2);\n//ctx2.lineTo(j * 1, canvas2.height - point);\n//ctx2.lineTo(j * 10, point);\n});\nctx2.stroke();\n});\n// ctx2.beginPath();\n// ctx2.strokeStyle = '';\n// ctx2.lineWidth = 1.5;\n// ctx2.lineTo(j * 1.33, (canvas2.height - point +2)  > 150?148:canvas2.height - point +2);\n// ctx2.stroke();\n// draw the labels\n// ctx2.fillStyle = 'white';\n// ctx2.fillText(data1.labels[data1.labels.length - 1], canvas2.width - 50, canvas2.height - 10);\n}\nconst canvasAbs = document.getElementById('graphCanvas');\nconst ctxAbs = canvasAbs.getContext('2d');\nconst width = canvasAbs.width;\nconst height = canvasAbs.height;\n// Grid Configuration: Split the canvas into 2x2 grid\nconst gridSize = 2; // 2x2 grid\nconst cellWidth = width / gridSize;\nconst cellHeight = height / gridSize;\nlet dataAbs = [\n[[[], []], [[], []]], // Top-left grid (2 lines)\n[[[], []], [[], []]], // Top-right grid (2 lines)\n[[[], []], [[], []]], // Bottom-left grid (2 lines)\n[[[], []], [[], []]], // Bottom-right grid (2 lines)\n];\n// Maximum number of points to display (adjust as needed)\nconst maxPoints = 300;\nconst pointSpacing = cellWidth / maxPoints;\n// Function to draw the grid lines\nfunction drawGrid() {\nctxAbs.strokeStyle = 'white';\nctxAbs.lineWidth = 1;\n// Draw vertical lines\nfor (let i = 1; i < gridSize; i++) {\nctxAbs.beginPath();\nctxAbs.moveTo(i * cellWidth, 0);\nctxAbs.lineTo(i * cellWidth, height);\nctxAbs.stroke();\n}\n// Draw horizontal lines\nfor (let i = 1; i < gridSize; i++) {\nctxAbs.beginPath();\nctxAbs.moveTo(0, i * cellHeight);\nctxAbs.lineTo(width, i * cellHeight);\nctxAbs.stroke();\n}\n}\nfunction drawGraph() {\nconst padding = 5; // Add padding to prevent overlap with borders\nfor (let i = 0; i < gridSize; i++) {\nfor (let j = 0; j < gridSize; j++) {\nconst xOffset = j * cellWidth + padding;      // Add padding to the x offset\nconst yOffset = i * cellHeight + padding;     // Add padding to the y offset\nconst usableWidth = cellWidth - 2 * padding;  // Adjust usable width inside the cell\nconst usableHeight = cellHeight - 2 * padding;// Adjust usable height inside the cell\n// Loop over each data line for this quadrant (e.g., 2 lines per quadrant)\nfor (let line = 0; line < dataAbs[i][j].length; line++) {\n// Set color for each line\nctxAbs.strokeStyle = (line === 0) ? 'cyan' : 'red';  // Use different colors for each line\nctxAbs.lineWidth = 1.2;\nctxAbs.beginPath(); // Begin one path per line\n// Move to the first point (scaled by percentage)\nif (dataAbs[i][j][line].length > 0) {\nconst firstPoint = yOffset + usableHeight - (dataAbs[i][j][line][0] / 100 * usableHeight);\nctxAbs.moveTo(xOffset, firstPoint);\n}\n// Line to subsequent points (scaled by percentage)\nfor (let k = 1; k < dataAbs[i][j][line].length; k++) {\nconst x = xOffset + k * (usableWidth / maxPoints);\nconst y = yOffset + usableHeight - (dataAbs[i][j][line][k] / 100 * usableHeight);\nctxAbs.lineTo(x, y);\n}\n// Stroke the path once per line\nctxAbs.stroke();\n}\n}\n}\n}\nfunction animate() {\nctxAbs.clearRect(0, 0, width, height); // Clear the canvas\ndrawGrid();                         // Draw the grid\ndrawGraph();                        // Draw the graph\n}\nfor (let i = 0; i < relativeTrs.length; i++) {\nrelativeTds[i] = relativeTrs[i].querySelectorAll(\"td\");\n}\ndocument.querySelector(\"#toggle-settings\").addEventListener( \"click\", (e) => {\naddSettingsListeners();\n} )\ndocument.querySelector(\"#reload-page\").addEventListener( \"click\", (e) => {\nwindow.location.reload();\n} )\nconst canvas = document.getElementById('myCanvas');\nconst ctx = canvas.getContext('2d');\nvar carMain = { x: canvas.width / 2, y: canvas.height / 2, rotation: 0, width: 1.970000*20, length: 4.756699*20 };\nvar cars = [];\nlet canvasWidth, canvasHeight;\nfunction updateCanvasSize(width, height) {\n// canvasWidth = canvas.width = width;\n// canvasHeight = canvas.height = height;\n// requestAnimationFrame(drawCars);\n}\nfunction drawCar(x, y, rotation, width, length, color) {\nctx.save();\nctx.translate(x, y);\nctx.rotate(rotation);\nctx.fillStyle = color;\nctx.fillRect(-width / 2, -length / 2, width, length);\nctx.beginPath();\nctx.moveTo(0, 0);\nif (color == 'red') {\nctx.lineTo(0, length / 2);\nctx.strokeStyle = 'black';\nctx.stroke();\n} else {\nctx.lineTo(0, -length / 2);\nctx.strokeStyle = 'black';\nctx.stroke();\nctx.strokeStyle = 'red';\nctx.beginPath();\nctx.moveTo(0, 0);\nctx.lineTo(length/2 * Math.cos(chv[0]), length/2 * Math.sin(chv[0]));\nctx.stroke();\nctx.strokeStyle = 'green';\nctx.beginPath();\nctx.moveTo(0, 0);\nctx.lineTo(length/2 * Math.cos(chv[1]), length/2 * Math.sin(chv[1]));\nctx.stro";
char* responz2 = "ke();\n}\nctx.restore();\n}\nfunction drawCars() {\nctx.clearRect(0, 0, canvas.width, canvas.height);\ndrawCar(carMain.x, carMain.y, carMain.rotation, carMain.width, carMain.length, 'white');\nif ( cars.length > 0 )\nfor (let i = 0; i < cars.length; i++) {\ndrawCar(cars[i].x, cars[i].y, cars[i].rotation, cars[i].width, cars[i].length, 'red');\n}\n}\nfunction printFrame() {\ndrawCars();\n}\nupdateCanvasSize(400,400);\n//window.addEventListener('resize', updateCanvasSize);\nrequestAnimationFrame(drawCars);\nvar resizableTable = null;\nvar draggableCorner = null;\nlet isDragging = false;\nlet isResizing = false;\nlet isScaling = false;\nlet initialX, initialY, initialWidth, initialHeight, initialScale;\nvar isSettingsOn = false;\nfunction msDwn ( e ) {\ne.preventDefault();\nif (/*e.target.id == \"draggable-corner\" &&*/ e.button == 2) {\nswitch (e.button) {\ncase 0:\nresizableTable = e.target.parentElement;\nisResizing = true;\ninitialWidth = resizableTable.offsetWidth;\ninitialHeight = resizableTable.offsetHeight;\ninitialX = e.clientX;\ninitialY = e.clientY;\nbreak;\ncase 2:\nresizableTable = e.target;\nwhile (resizableTable.id !== 'resizable-table') {\nresizableTable = resizableTable.parentElement;\n}\n//resizableTable = e.target.parentElement.firstElementChild;\nresizableTable = resizableTable.firstElementChild;\nisScaling = true;\nvar style = window.getComputedStyle(resizableTable);\nvar transform = style.getPropertyValue('transform');\nif (transform != 'none') {\nvar values = transform.split('(')[1].split(')')[0].split(',');\ninitialScale = parseFloat(values[0]);\n} else {\ninitialScale = 1;\n}\ninitialX = e.clientX;\ninitialY = e.clientY;\nbreak;\ndefault:\nconsole.log('Unexpected mouse button clicked.');\n}\n} else {\nresizableTable = e.target;\nwhile (resizableTable.id !== 'resizable-table') {\nresizableTable = resizableTable.parentElement;\n}\nisDragging = true;\ninitialX = e.clientX - resizableTable.offsetLeft;\ninitialY = e.clientY - resizableTable.offsetTop;\n}\n}\nfunction msMv ( e ) {\nif (isScaling) {\nvar movementX = e.clientX - initialX;\nvar newScale = initialScale + movementX / 100;\nresizableTable.style.transform = 'scale(' + newScale + ')';\n}/* else if (isResizing) {\nconst deltaX = e.clientX - initialX;\nconst deltaY = e.clientY - initialY;\nconst newWidth = Math.max(30, initialWidth + deltaX);\nconst newHeight = Math.max(30, initialHeight + deltaY);\nconst maxX = window.innerWidth - resizableTable.offsetLeft;\nconst maxY = window.innerHeight - resizableTable.offsetTop;\nresizableTable.style.width = `${Math.min(newWidth, maxX)}px`;\nresizableTable.style.height = `${Math.min(newHeight, maxY)}px`;\nif ( resizableTable.className === \"canv\" ) {\nupdateCanvasSize(Math.min(newWidth, maxX), Math.min(newHeight, maxY));\n}\n}*/ else if (isDragging) {\nconst newX = e.clientX - initialX;\nconst newY = e.clientY - initialY;\n// const maxX = window.innerWidth - resizableTable.offsetWidth;\n// const maxY = window.innerHeight - resizableTable.offsetHeight;\nresizableTable.style.left = `${newX}px`;\nresizableTable.style.top = `${newY}px`;\n// const clampedX = Math.max(0, Math.min(newX, maxX));\n// const clampedY = Math.max(0, Math.min(newY, maxY));\n// resizableTable.style.left = `${clampedX}px`;\n// resizableTable.style.top = `${clampedY}px`;\n}\n}\nfunction msUp ( e ) {\nisDragging = false;\nisResizing = false;\nisScaling = false;\n}\nfunction sendPositions () {\nvar firstLeft = -1000;\nvar dataToSend = [9];\nvar count = 0\ndocument.querySelectorAll(\"#resizable-table\").forEach(function (element) {\nconsole.log(element)\nvar rect = element.getBoundingClientRect();\nif (firstLeft == -1000) {\nfirstLeft = rect.left;\n}\ndataToSend.push(Math.round(rect.left));\ndataToSend.push(Math.round(rect.top));\nvar resizableTable = element.firstElementChild;\nvar style = window.getComputedStyle(resizableTable);\nvar transform = style.getPropertyValue('transform');\nif (transform != 'none') {\nvar values = transform.split('(')[1].split(')')[0].split(',');\nvar trans = parseFloat(values[0]);\ndataToSend.push(Math.floor(trans));\ndataToSend.push(Math.round((trans-Math.floor(trans))*100).toFixed(0));\n// dataToSend.push(Math.round((trans-Math.floor(trans))*100));\n} else {\ndataToSend.push(1);\ndataToSend.push(0);\n}\ncount++;\n});\nvar byteArray = new Uint16Array(dataToSend);\nlet char1 = firstLeft & 0xFF;  // Low byte\nlet char2 = (firstLeft >> 8) & 0xFF;\nvar buff = byteArray.buffer;\nvar viewTemp = new Uint8Array(buff);\nconsole.log(viewTemp)\nviewTemp[2] = char1;\nviewTemp[3] = char2;\nconst dataView = new DataView(buff);\n// Function to get signed 8-bit integers\nfunction getSignedInt8(view, byteOffset) {\nreturn view.getInt8(byteOffset);\n}\nvar firstBuff = buff.slice(0, 98);\nvar secondBuff = buff.slice(98);\nvar modifiedSecondBuff = new Uint8Array(secondBuff.byteLength + 2);\nmodifiedSecondBuff[0] = 99;\nmodifiedSecondBuff[1] = 0;\nmodifiedSecondBuff.set(new Uint8Array(secondBuff), 2);\nconsole.log(modifiedSecondBuff)\nvar viewCorrected = new Uint16Array(firstBuff);\nsocket.send(viewCorrected.buffer);\nviewCorrected = new Uint16Array(modifiedSecondBuff.buffer);\nsocket.send(viewCorrected.buffer);\nconsole.log(viewCorrected)\nconsole.log(secondBuff)\n}\nfunction addSettingsListeners () {\nif ( isSettingsOn === true ) {\ndocument.querySelectorAll(\"#resizable-table\").forEach(function (element) {\nelement.removeEventListener(\"mousedown\", msDwn);\nelement.querySelector(\"#draggable-corner\").classList.add('hide');\n//element.classList.remove(\"border\");\n});\ndocument.querySelector(\"#startLight\").classList.add('hide');\ndocument.removeEventListener(\"mousemove\", msMv);\ndocument.removeEventListener(\"mouseup\", msUp);\nisSettingsOn = false;\nsendPositions();\n} else {\ndocument.querySelectorAll(\"#resizable-table\").forEach(function (element) {\nelement.addEventListener(\"mousedown\", msDwn);\nelement.querySelector(\"#draggable-corner\").classList.remove('hide');\n//element.classList.add(\"border\");\n});;\ndocument.querySelector(\"#startLight\").classList.remove('hide');\ndocument.addEventListener(\"mousemove\", msMv);\ndocument.addEventListener(\"mouseup\", msUp);\nisSettingsOn = true;\n}\n}\nfl = document.querySelectorAll(\"#wheels td\")[15]\nfr = document.querySelectorAll(\"#wheels td\")[16];\nrl = document.querySelectorAll(\"#wheels td\")[23];\nrr = document.querySelectorAll(\"#wheels td\")[24];\nsocket.binaryType = \"arraybuffer\";\nsocket.addEventListener('open', (event) => {\n});\nvar delay = 0;\nsocket.addEventListener('message', (event) => {\nif (event.data instanceof ArrayBuffer) {\nlet view = new Uint8Array(event.data);\nconst view1 = new DataView(event.data);\nif (view[0] == 111) {\nif (view1.byteLength > 10) {\nvar bytes = 1;\nwhile ( true ) {\nif ( bytes == view1.byteLength ) {\nbreak;\n}\ncars.push({\nx: -view1.getFloat32(bytes, true)+500,\ny: -view1.getFloat32(bytes + 4, true)+500,\nrotation: view1.getFloat32(bytes + 8, true),\nwidth: view1.getFloat32(bytes + 12, true)*20,\nlength: view1.getFloat32(bytes + 16, true)*20\n});\nbytes += 20\n}\n}\nreturn;\n}\nif (view[0] == 101 && slipGraphSetts) {\nvar offset = 1;\nvar fl = view1.getFloat32(offset, true);\noffset += 4;\nvar rl = view1.getFloat32(offset, true);\noffset += 4;\nchv[0] = view1.getFloat32(offset, true);\noffset += 4;\nchv[1] = view1.getFloat32(offset, true);\noffset += 4;\n//chv[2] = view1.getInt8(offset, true);\ndata1.labels.push(new Date().toLocaleTimeString());\ndata1.datasets[0].data.push(fl * canvas1.height);\ndata1.datasets[1].data.push(rl * canvas1.height);\ndata2.datasets[0].data.push(view1.getInt8(offset, true));\n// if (data.labels.length > canvas1.width / 1) {\nif (data1.labels.length > 300) {\ndata1.labels.shift();\ndata1.datasets.forEach(dataset => dataset.data.shift());\ndata2.datasets.forEach(dataset => dataset.data.shift());\n}\ndrawChartSlip();\nreturn;\n}\nif (view[0] == 1) {\nif (radarSetts) {\nvar deltaHtml = view1.getFloat32(1, true).toFixed(3)\nif (deltaHtml > 0) {\ndelta.style.color = `rgb(255,0,0)`;\n} else {\ndelta.style.color = `rgb(0,255,0)`;\n}\ndeltaHtml = deltaHtml == -1000 ? \"-\" : deltaHtml\ndelta.innerHTML = deltaHtml;\ncarMain.x = -view1.getFloat32(1 + 4, true)+500;\ncarMain.y = -view1.getFloat32(1 + 8, true)+500;\ncarMain.rotation = view1.getFloat32(1 + 12, true);\ncarMain.width = view1.getFloat32(1 + 16, true)*20;\ncarMain.length = view1.getFloat32(1 + 20, true)*20;\nvar bytes = 21\nwhile ( true ) {\nif ( bytes + 4 == view1.byteLength ) {\nbreak;\n}\ncars.push({\nx: -view1.getFloat32(bytes + 4, true)+500,\ny: -view1.getFloat32(bytes + 8, true)+500,\nrotation: view1.getFloat32(bytes + 12, true),\nwidth: view1.getFloat32(bytes + 16, true)*20,\nlength: view1.getFloat32(bytes + 20, true)*20\n});\nbytes += 20\n}\nfunction calculateDistance(carMain, car2) {\nvar dx = car2.x - carMain.x;\nvar dy = car2.y - carMain.y;\nvar distanceCenter = Math.sqrt(dx * dx + dy * dy);\ndistanceCenter *=20;\nreturn distanceCenter;\n}\nfunction calculateSecondCarPosition(carMain, distance, angle) {\nconst newX = carMain.x - distance * Math.cos(angle);\nconst newY = carMain.y + distance * Math.sin(angle);\nreturn { x: newX, y: newY };\n}\nfunction calculateAngle(carMain, car2) {\nreturn Math.atan2(car2.y - carMain.y, car2.x - carMain.x);\n}\nvar dist = 0;\nif ( cars.length > 0 )\nfor (let i = 0; i < cars.length; i++) {\ncarMainTemp = { ...carMain };\nconst angleBetweenCars = calculateAngle(carMainTemp, cars[i]);\ndistance = calculateDistance(carMainTemp, cars[i])\ndist = distance\nvar angle = angleBetweenCars;\nlet equivalentAngle1 = ((angle * (180 / Math.PI)) + (carMainTemp.rotation * (180 / Math.PI)))  % 360;\nif (equivalentAngle1 < 0) {\nequivalentAngle1 += 360;\n}\nangle = equivalentAngle1 - 180;\nangle = (angle * Math.PI) / 180\ncarMainTemp.x = canvas.width / 2;\ncarMainTemp.y = canvas.height / 2;\nvar pso = calculateSecondCarPosition(carMainTemp, distance, angle);\ncars[i].x = pso.x;\ncars[i].y = pso.y;\ncarMainTemp.rotation = carMainTemp.rotation * (180 / Math.PI);\ncars[i].rotation = cars[i].rotation * (180 / Math.PI);\ncarMainTemp.rotation = 360 - carMainTemp.rotation;\nlet sum = carMainTemp.rotation + cars[i].rotation;\nlet equivalentAngle = sum % 360;\nif (equivalentAngle < 0) {\nequivalentAngle += 360;\n}\ncars[i].rotation = equivalentAngle - 180;\ncarMainTemp.rotation = (360 * Math.PI) / 180;\ncars[i].rotation = (cars[i].rotation * Math.PI) / 180;\n}\ncarMain.x = canvas.width / 2;\ncarMain.y = canvas.height / 2;\ncarMain.rotation = (360 * Math.PI) / 180;\nprintFrame();\ncars.length = 0;\n}\nreturn;\n}\nif (view[0] == 2) {\nif (wheelsSetts) {\nvar offset = 1;\nflWear.innerHTML = (view1.getFloat32(offset, true)*100).toFixed(2);\noffset += 4;\nfrWear.innerHTML = (view1.getFloat32(offset, true)*100).toFixed(2);\noffset += 4;\nrlWear.innerHTML = (view1.getFloat32(offset, true)*100).toFixed(2);\noffset += 4;\nrrWear.innerHTML = (view1.getFloat32(offset, true)*100).toFixed(2);\noffset += 4;\nflPressure.innerHTML = view1.getFloat32(offset, true).toFixed(1);\noffset += 4;\nfrPressure.innerHTML = view1.getFloat32(offset, true).toFixed(1);\noffset += 4;\nrlPressure.innerHTML = view1.getFloat32(offset, true).toFixed(1);\noffset += 4;\nrrPressure.innerHTML = view1.getFloat32(offset, true).toFixed(1);\noffset += 4;\nflDirt.innerHTML = (view1.getFloat32(offset, true)*100).toFixed(3);\noffset += 4;\nfrDirt.innerHTML = (view1.getFloat32(offset, true)*100).toFixed(3);\noffset += 4;\nrlDirt.innerHTML = (view1.getFloat32(offset, true)*100).toFixed(3);\noffset += 4;\nrrDirt.innerHTML = (view1.getFloat32(offset, true)*100).toFixed(3);\noffset += 4;\nvar flBrakeTemp = view1.getFloat32(offset, true).toFixed(0);\nflBrake.innerHTML = flBrakeTemp;\noffset += 4;\nvar brakeTempOptFl = view1.getFloat32(offset, true);\noffset += 4;\nvar brakeTempCldFl = view1.getFloat32(offset, true);\noffset += 4;\nvar brakeTempHotFl = view1.getFloat32(offset, true);\noffset += 4;\nflBrake.parentElement.parentElement.style.backgroundColor = getTemperatureColor(flBrakeTemp, brakeTempCldFl, brakeTempOptFl, brakeTempHotFl);\nvar frBra";
char* responz3 = "keTemp = view1.getFloat32(offset, true).toFixed(0);\nfrBrake.innerHTML = frBrakeTemp;\noffset += 4;\nvar brakeTempOptFr = view1.getFloat32(offset, true);\noffset += 4;\nvar brakeTempCldFr = view1.getFloat32(offset, true);\noffset += 4;\nvar brakeTempHotFr = view1.getFloat32(offset, true);\noffset += 4;\nfrBrake.parentElement.parentElement.style.backgroundColor = getTemperatureColor(frBrakeTemp, brakeTempCldFr, brakeTempOptFr, brakeTempHotFr);\nvar rlBrakeTemp = view1.getFloat32(offset, true).toFixed(0);\nrlBrake.innerHTML = rlBrakeTemp;\noffset += 4;\nvar brakeTempOptRl = view1.getFloat32(offset, true);\noffset += 4;\nvar brakeTempCldRl = view1.getFloat32(offset, true);\noffset += 4;\nvar brakeTempHotRl = view1.getFloat32(offset, true);\noffset += 4;\nrlBrake.parentElement.parentElement.style.backgroundColor = getTemperatureColor(rlBrakeTemp, brakeTempCldRl, brakeTempOptRl, brakeTempHotRl);\nvar rrBrakeTemp = view1.getFloat32(offset, true).toFixed(0);\nrrBrake.innerHTML = rrBrakeTemp;\noffset += 4;\nvar brakeTempOptRr = view1.getFloat32(offset, true);\noffset += 4;\nvar brakeTempCldRr = view1.getFloat32(offset, true);\noffset += 4;\nvar brakeTempHotRr = view1.getFloat32(offset, true);\noffset += 4;\nrrBrake.parentElement.parentElement.style.backgroundColor = getTemperatureColor(rrBrakeTemp, brakeTempCldRr, brakeTempOptRr, brakeTempHotRr);\n}\nreturn;\n}\nif (view[0] == 3) {\nvar offset = 1;\nvar flTemp1Temp = view1.getFloat32(offset, true).toFixed(0);\nflTemp1.innerHTML = flTemp1Temp;\noffset += 4;\nvar flTemp2Temp = view1.getFloat32(offset, true).toFixed(0);\nflTemp2.innerHTML = flTemp2Temp;\noffset += 4;\nvar flTemp3Temp = view1.getFloat32(offset, true).toFixed(0);\nflTemp3.innerHTML = flTemp3Temp;\noffset += 4;\nvar tireTempOptFl = view1.getFloat32(offset, true);\noffset += 4;\nvar tireTempCldFl = view1.getFloat32(offset, true);\noffset += 4;\nvar tireTempHotFl = view1.getFloat32(offset, true);\noffset += 4;\nflTemp1.parentElement.parentElement.style.backgroundColor = getTemperatureColor(flTemp1Temp, tireTempCldFl, tireTempOptFl, tireTempHotFl);\nflTemp2.parentElement.parentElement.style.backgroundColor = getTemperatureColor(flTemp2Temp, tireTempCldFl, tireTempOptFl, tireTempHotFl);\nflTemp3.parentElement.parentElement.style.backgroundColor = getTemperatureColor(flTemp3Temp, tireTempCldFl, tireTempOptFl, tireTempHotFl);\nvar frTemp1Temp = view1.getFloat32(offset, true).toFixed(0);\nfrTemp1.innerHTML = frTemp1Temp;\noffset += 4;\nvar frTemp2Temp = view1.getFloat32(offset, true).toFixed(0);\nfrTemp2.innerHTML = frTemp2Temp;\noffset += 4;\nvar frTemp3Temp = view1.getFloat32(offset, true).toFixed(0);\nfrTemp3.innerHTML = frTemp3Temp;\noffset += 4;\nvar tireTempOptFr = view1.getFloat32(offset, true);\noffset += 4;\nvar tireTempCldFr = view1.getFloat32(offset, true);\noffset += 4;\nvar tireTempHotFr = view1.getFloat32(offset, true);\noffset += 4;\nfrTemp1.parentElement.parentElement.style.backgroundColor = getTemperatureColor(frTemp1Temp, tireTempCldFr, tireTempOptFr, tireTempHotFr);\nfrTemp2.parentElement.parentElement.style.backgroundColor = getTemperatureColor(frTemp2Temp, tireTempCldFr, tireTempOptFr, tireTempHotFr);\nfrTemp3.parentElement.parentElement.style.backgroundColor = getTemperatureColor(frTemp3Temp, tireTempCldFr, tireTempOptFr, tireTempHotFr);\nvar rlTemp1Temp = view1.getFloat32(offset, true).toFixed(0);\nrlTemp1.innerHTML = rlTemp1Temp;\noffset += 4;\nvar rlTemp2Temp = view1.getFloat32(offset, true).toFixed(0);\nrlTemp2.innerHTML = rlTemp2Temp;\noffset += 4;\nvar rlTemp3Temp = view1.getFloat32(offset, true).toFixed(0);\nrlTemp3.innerHTML = rlTemp3Temp;\noffset += 4;\nvar tireTempOptRl = view1.getFloat32(offset, true);\noffset += 4;\nvar tireTempCldRl = view1.getFloat32(offset, true);\noffset += 4;\nvar tireTempHotFRl = view1.getFloat32(offset, true);\noffset += 4;\nrlTemp1.parentElement.parentElement.style.backgroundColor = getTemperatureColor(rlTemp1Temp, tireTempCldRl, tireTempOptRl, tireTempHotFRl);\nrlTemp2.parentElement.parentElement.style.backgroundColor = getTemperatureColor(rlTemp2Temp, tireTempCldRl, tireTempOptRl, tireTempHotFRl);\nrlTemp3.parentElement.parentElement.style.backgroundColor = getTemperatureColor(rlTemp3Temp, tireTempCldRl, tireTempOptRl, tireTempHotFRl);\nvar rrTemp1Temp = view1.getFloat32(offset, true).toFixed(0);\nrrTemp1.innerHTML = rrTemp1Temp;\noffset += 4;\nvar rrTemp2Temp = view1.getFloat32(offset, true).toFixed(0);\nrrTemp2.innerHTML = rrTemp2Temp;\noffset += 4;\nvar rrTemp3Temp = view1.getFloat32(offset, true).toFixed(0);\nrrTemp3.innerHTML = rrTemp3Temp;\noffset += 4;\nvar tireTempOptRr = view1.getFloat32(offset, true);\noffset += 4;\nvar tireTempCldRr = view1.getFloat32(offset, true);\noffset += 4;\nvar tireTempHotFRr = view1.getFloat32(offset, true);\noffset += 4;\nrrTemp1.parentElement.parentElement.style.backgroundColor = getTemperatureColor(rrTemp1Temp, tireTempCldRr, tireTempOptRr, tireTempHotFRr);\nrrTemp2.parentElement.parentElement.style.backgroundColor = getTemperatureColor(rrTemp2Temp, tireTempCldRr, tireTempOptRr, tireTempHotFRr);\nrrTemp3.parentElement.parentElement.style.backgroundColor = getTemperatureColor(rrTemp3Temp, tireTempCldRr, tireTempOptRr, tireTempHotFRr);\nreturn;\n}\nif ( view[0] == 4 && relativeTableSetts) {\nvar ratrep;\nvar offset  = 1;\nvar fuelLeft = view1.getFloat32(offset, true);\noffset += 4;\nfuelMonitorTds[5].innerHTML = fuelLeft.toFixed(2);\nvar fuelForRace = view1.getFloat32(offset, true);\noffset += 4;\nfuelMonitorTds[4].innerHTML = fuelForRace;\n// console.log(view1.getInt32(offset, true));\n// console.log(loadedRat);\n// console.log(loadedRat);\ntry {\nrelativeTds[5][1].innerHTML = loadedRat[view1.getInt32(offset, true)].Fullname;\nratrep = loadedRat[view1.getInt32(offset, true)].Rating + \"/\" + loadedRat[view1.getInt32(offset, true)].Reputation;\noffset += 4;\nrelativeTds[5][0].innerHTML = view1.getInt32(offset, true);\noffset += 4;\nrelativeTds[5][2].innerHTML = ratrep;\nfor (let index = 4; index > -1; index--) {\nif ( view1.byteLength > offset && view1.getInt32(offset, true) != 0 ) {\nrelativeTds[index][1].innerHTML = loadedRat[view1.getInt32(offset, true)].Fullname;\nratrep = loadedRat[view1.getInt32(offset, true)].Rating + \"/\" + loadedRat[view1.getInt32(offset, true)].Reputation;\noffset += 4;\nrelativeTds[index][0].innerHTML = view1.getInt8(offset, true);\noffset += 1;\nrelativeTds[index][2].innerHTML = ratrep;\n// relativeTds[index][3].innerHTML = view1.getInt8(offset, true)//float16ToFloat32(view1.getUint16(offset, true)).toFixed(3)//float16ToFloat32(view1.getUint16(offset, true))>-999&&float16ToFloat32(view1.getUint16(offset, true))<999?float16ToFloat32(view1.getUint16(offset, true)).toFixed(3):(float16ToFloat32(view1.getUint16(offset, true))/1000).toFixed(0);\n// offset += 1;\nrelativeTds[index][3].innerHTML = float16ToFloat32(view1.getUint16(offset, true)).toFixed(3)//float16ToFloat32(view1.getUint16(offset, true))>-999&&float16ToFloat32(view1.getUint16(offset, true))<999?float16ToFloat32(view1.getUint16(offset, true)).toFixed(3):(float16ToFloat32(view1.getUint16(offset, true))/1000).toFixed(0);\noffset += 2;\nif ( view1.getInt8(offset, true) == -1 && relativeTds[index][1].style.backgroundColor != '#656565') {\nrelativeTds[index][1].style.backgroundColor = '#656565';\n}else\nif ( (view1.getInt8(offset, true) == 0 || view1.getInt8(offset, true) == 1 || view1.getInt8(offset, true) == 2) && relativeTds[index][1].style.backgroundColor != '#db5555' ) {\nrelativeTds[index][1].style.backgroundColor = '#db5555';\n}else\nif ( view1.getInt8(offset, true) == 4 && relativeTds[index][1].style.backgroundColor != '#f7e372' ) {\nrelativeTds[index][1].style.backgroundColor = '#f7e372';\n}else\nif ( view1.getInt8(offset, true) == 5 && relativeTds[index][1].style.backgroundColor != '#000' ) {\nrelativeTds[index][1].style.backgroundColor = '#000';\n}\noffset += 1;\nvar value = view1.getInt8(offset, true);\nvar currentBgColor = relativeTds[index][3].style.background;\nif ( value == 0 && currentBgColor != '#656565') {\nrelativeTds[index][3].style.background = ''\nrelativeTds[index][3].style.backgroundColor = '#656565';\n} else\nif ( value < 0 && currentBgColor != '#0E4FDD') {\nrelativeTds[index][3].style.background = ''\nrelativeTds[index][3].style.backgroundColor = '#0E4FDD';\n} else\nif ( value > 0 && currentBgColor != 'linear-gradient(to bottom, #0D802F 70%, #0E4FDD)') {\nrelativeTds[index][3].style.backgroundColor = '';\nrelativeTds[index][3].style.background = 'linear-gradient(to bottom, #0D802F 70%, #0E4FDD)';\n}\noffset += 1;\n} else {\nrelativeTds[index][0].innerHTML = '';\nrelativeTds[index][1].innerHTML = '';\nrelativeTds[index][2].innerHTML = '';\nrelativeTds[index][3].innerHTML = '';\noffset += 9;\n}\n}\nfor (let index = 6; index < 11; index++) {\n// console.log(offset)\n// console.log(view1.getInt32(offset, true))\n// console.log(view1.getInt8(offset, true))\n// console.log(view1.getInt8(offset+1, true))\n// console.log(view1.getInt8(offset+2, true))\n// console.log(view1.getInt8(offset+3, true))\nif ( view1.byteLength > offset && view1.getInt32(offset, true) != 0 ) {console.log(view1.getInt32(offset, true))\nrelativeTds[index][1].innerHTML = loadedRat[view1.getInt32(offset, true)].Fullname;\nratrep = loadedRat[view1.getInt32(offset, true)].Rating + \"/\" + loadedRat[view1.getInt32(offset, true)].Reputation;\noffset += 4;\nrelativeTds[index][0].innerHTML = view1.getInt8(offset, true);\noffset += 1;\nrelativeTds[index][2].innerHTML = ratrep;\n// relativeTds[index][3].innerHTML = view1.getInt8(offset, true)//float16ToFloat32(view1.getUint16(offset, true)).toFixed(3)//float16ToFloat32(view1.getUint16(offset, true))>-999&&float16ToFloat32(view1.getUint16(offset, true))<999?float16ToFloat32(view1.getUint16(offset, true)).toFixed(3):(float16ToFloat32(view1.getUint16(offset, true))/1000).toFixed(0);\n// offset += 1;\nrelativeTds[index][3].innerHTML = float16ToFloat32(view1.getUint16(offset, true)).toFixed(3)//float16ToFloat32(view1.getUint16(offset, true))>-999&&float16ToFloat32(view1.getUint16(offset, true))<999?float16ToFloat32(view1.getUint16(offset, true)).toFixed(3):(float16ToFloat32(view1.getUint16(offset, true))/1000).toFixed(0);\noffset += 2;\nif ( view1.getInt8(offset, true) == -1 && relativeTds[index][1].style.backgroundColor != '#656565') {\nrelativeTds[index][1].style.backgroundColor = '#656565';\n}else\nif ( (view1.getInt8(offset, true) == 0 || view1.getInt8(offset, true) == 1 || view1.getInt8(offset, true) == 2) && relativeTds[index][1].style.backgroundColor != '#db5555' ) {\nrelativeTds[index][1].style.backgroundColor = '#db5555';\n}else\nif ( view1.getInt8(offset, true) == 4 && relativeTds[index][1].style.backgroundColor != '#f7e372' ) {\nrelativeTds[index][1].style.backgroundColor = '#f7e372';\n}else\nif ( view1.getInt8(offset, true) == 5 && relativeTds[index][1].style.backgroundColor != '#000' ) {\nrelativeTds[index][1].style.backgroundColor = '#000';\n}\noffset += 1;\nvar value = view1.getInt8(offset, true);\nvar currentBgColor = relativeTds[index][3].style.background;\nif ( value == 0 && currentBgColor != '#656565') {\nrelativeTds[index][3].style.background = ''\nrelativeTds[index][3].style.backgroundColor = '#656565';\n} else\nif ( value < 0 && currentBgColor != '#0E4FDD') {\nrelativeTds[index][3].style.background = ''\nrelativeTds[index][3].style.backgroundColor = '#0E4FDD';\n} else\nif ( value > 0 && currentBgColor != 'linear-gradient(to bottom, #0D802F 70%, #0E4FDD)') {\nrelativeTds[index][3].style.backgroundColor = '';\nrelativeTds[index][3].style.background = 'linear-gradient(to bottom, #0D802F 70%, #0E4FDD)';\n}\noffset += 1;\n} else {\nrelativeTds[index][0].innerHTML = '';\nrelativeTds[index][1].innerHTML = '';\nrelativeTds[index][2].innerHTML = '';\nrelativeTds[index][3].innerHTML = '';\noffset += 9;\n}\n}\n} catch (err) {console.log(err)}\nreturn;\n}\nif ( view[0] == 5) {\nlet decoder = new TextDecoder('utf-8');\nlet str = decoder.decode(new DataView(event.data, 1, view1.byteLength-2));\nlet arr = str.split(',');\nlet obj = {\n\"UserId\": arr[0],\n\"Fullname\" : arr[1],\n";
char* responz4 = "\"Rating\": arr[2],\n\"Reputation\": arr[3]\n};\nloadedRat[arr[0]] = obj;\nreturn;\n}\nif ( view[0] == 6 ) {\nvar offset = 1;\nvar lastLapHtml = view1.getFloat32(offset, true);\nif (lastLapHtml == 9999) {\nlastLap.innerHTML = \"-:--.---\";\n} else {\nvar res = Math.floor(lastLapHtml / 60);\nif ( res >= 1 ) {\nlastLap.innerHTML = res+\":\"+(lastLapHtml - 60 * res).toFixed(3).padStart(6, '0');\n} else {\nlastLap.innerHTML = lastLapHtml.toFixed(3).padStart(6, '0');\n}\n}\nif ( view1.byteLength < 6 ) {\nreturn;\n}\noffset += 4;\nvar bestLapHtml= view1.getFloat32(offset, true);\nif (bestLapHtml == 9999/* || bestLapHtml == -1 */) {\nbestLap.innerHTML = \"-:--.---\";\n} else {\nvar res = Math.floor(bestLapHtml / 60);\nif ( res >= 1 ) {\nbestLap.innerHTML = res+\":\"+(bestLapHtml - 60 * res).toFixed(3).padStart(6, '0');\n} else {\nbestLap.innerHTML = bestLapHtml.toFixed(3).padStart(6, '0');\n}\n}\noffset += 4;\nvar bestLapLeadHtml = view1.getFloat32(offset, true);\nif (bestLapLeadHtml == 9999) {\nbestLapLead.innerHTML = \"-:--.---\";\n} else {\nvar res = Math.floor(bestLapLeadHtml / 60);\nif ( res >= 1 ) {\nbestLapLead.innerHTML = res+\":\"+(bestLapLeadHtml - 60 * res).toFixed(3).padStart(6, '0');\n} else {\nbestLapLead.innerHTML = bestLapLeadHtml.toFixed(3).padStart(6, '0');\n}\n}\nif ( view1.byteLength < 14 ) {\nreturn;\n}\noffset += 4;\nvar timeHtml = view1.getFloat32(offset, true);\nif (timeHtml == 9999) {\nfuelMonitorTds[3].innerHTML = \"-:--.---\";\n} else {\nvar res = Math.floor(timeHtml / 60);\nif ( res >= 1 ) {\nfuelMonitorTds[3].innerHTML = res+\":\"+(timeHtml - 60 * res).toFixed(3).padStart(6, '0');\n} else {\nfuelMonitorTds[3].innerHTML = timeHtml.toFixed(3).padStart(6, '0');\n}\n}\nif ( view1.byteLength < 16 ) {\nreturn;\n}\noffset += 4;\nvar fuelHtml = view1.getFloat32(offset, true);\nfuelHtml = fuelHtml == 9999 ? \"-\" : fuelHtml.toFixed(2);\nfuelMonitorTds[2].innerHTML = fuelHtml;\noffset += 4;\nreturn;\n}\nif ( view[0] == 7 ) {\nstartLight.classList.remove('hide');\nswitch (view1.getUint32(1, true)) {\ncase 0:\nbreak;\ncase 1:\nstartLightDivs[0].classList.remove('off');\nstartLightDivs[0].classList.add('red');\nbreak;\ncase 2:\nstartLightDivs[1].classList.remove('off');\nstartLightDivs[1].classList.add('red');\nbreak;\ncase 3:\nstartLightDivs[2].classList.remove('off');\nstartLightDivs[2].classList.add('red');\nbreak;\ncase 4:\nstartLightDivs[3].classList.remove('off');\nstartLightDivs[3].classList.add('red');\nbreak;\ncase 5:\nstartLightDivs[4].classList.remove('off');\nstartLightDivs[4].classList.add('red');\nbreak;\ncase 6:\nstartLightDivs[0].classList.add('off');\nstartLightDivs[0].classList.remove('red');\nstartLightDivs[1].classList.add('off');\nstartLightDivs[1].classList.remove('red');\nstartLightDivs[2].classList.add('off');\nstartLightDivs[2].classList.remove('red');\nstartLightDivs[3].classList.add('off');\nstartLightDivs[3].classList.remove('red');\nstartLightDivs[4].classList.add('off');\nstartLightDivs[4].classList.remove('red');\nstartLightDivs[5].classList.add('green');\nstartLightDivs[5].classList.remove('off');\nsetTimeout(function() {\nstartLight.classList.add('hide');\n}, 3000);\nbreak;\n}\nreturn;\n}\nif ( view[0] == 8 ) {\nlet id = view1.getUint32(1, true);\nsetTimeout(function() {\ntry {\nlet xhr = new XMLHttpRequest();\nxhr.open(\"GET\", 'https://game.raceroom.com/multiplayer-rating/user/'+id+'.json', true);\nxhr.onreadystatechange = function () {\nif (xhr.readyState == 4) {\nif (xhr.status == 200) {\nlet jsn = JSON.parse(xhr.responseText);\njsn.Rating = Math.floor(jsn.Rating);\njsn.Reputation = Math.floor(jsn.Reputation);\nlet parts = jsn.Fullname.split(' ');\nif (parts.length > 1) {\njsn.Fullname = parts[0].charAt(0) + '. ';// + parts[1];\nfor (let index = 1; index < parts.length; index++) {\njsn.Fullname += parts[index];\n}\n}\njsn.Fullname = jsn.Fullname.substring(0, 15);\nlet resultRat = \"1\"+jsn.UserId+\",\"+jsn.Fullname+\",\"+jsn.Rating+\",\"+jsn.Reputation;\nloadedRat[jsn.UserId] = {\n\"UserId\" : jsn.UserId,\n\"Fullname\" : jsn.Fullname,\n\"Rating\" : jsn.Rating,\n\"Reputation\" : jsn.Reputation,\n};\nsocket.send(resultRat);\n} else\nif (xhr.status == 404) {\nloadedRat[id] = {\n\"UserId\" : id,\n\"Fullname\" : \"None\",\n\"Rating\" : 0,\n\"Reputation\" : 0,\n};\nlet resultRat = \"1\"+id+\",None,0,0\";\nsocket.send(resultRat);\n}  else\nif (xhr.status == 0) {\nloadedRat[id] = {\n\"UserId\" : id,\n\"Fullname\" : \"None\",\n\"Rating\" : 0,\n\"Reputation\" : 0,\n};\nlet resultRat = \"1\"+id+\",None,0,0\";\nsocket.send(resultRat);\n} else {\nloadedRat[id] = {\n\"UserId\" : id,\n\"Fullname\" : \"None\",\n\"Rating\" : 0,\n\"Reputation\" : 0,\n};\n}\n}\n}\nxhr.send();\n} catch (err) {\n// loadedRat[id] = {\n//     \"UserId\" : id,\n//     \"Fullname\" : \"None\",\n//     \"Rating\" : 0,\n//     \"Reputation\" : 0,\n// };\n// let resultRat = \"1\"+id+\",None,0,0\";\n// socket.send(resultRat);\n}\n}, delay);\ndelay += 100;\nreturn;\n}\nif (view[0] == 9) {\nconst dataView = new DataView(event.data);\nif ( view1.byteLength < 10 ) {\ndocument.querySelector(\"body\").classList.remove('hide');\naddSettingsListeners();\n} else {\nvar ofst = 0;\nvar w,h = 0;\nvar count = 0;\ndocument.querySelectorAll(\"#resizable-table\").forEach(function (element) {\nif (count > 12)\nreturn;\ntry {\nvar style = \"\";\nstyle += \"left: \"+view1.getInt16(1 + ofst, true)+\"px; \";\nofst += 2\nstyle += \"top: \"+view1.getInt16(1 + ofst, true)+\"px; \";\nofst += 2\nvar d = view1.getInt16(1 + ofst, true);\nofst += 2\nvar f = (view1.getInt16(1 + ofst, true) / 100).toString();\nofst += 2\nelement.firstElementChild.style.cssText = \"transform: scale(\"+d+\".\"+f.split('.')[1]+\");\"\nelement.style.cssText = style;\n} catch (exp) {\n}\ncount++;\n});\ndocument.querySelector(\"body\").classList.remove('hide');\n}\nreturn;\n}\nif (view[0] == 99) {\nconst dataView = new DataView(event.data);\nvar ofst = 0;\nvar w,h = 0;\nvar count = 0;\ndocument.querySelectorAll(\"#resizable-table\").forEach(function (element) {\ncount++;\nif (count > 12){\ntry {\nvar style = \"\";\nstyle += \"left: \"+view1.getInt16(1 + ofst, true)+\"px; \";\nofst += 2\nstyle += \"top: \"+view1.getInt16(1 + ofst, true)+\"px; \";\nofst += 2\nvar d = view1.getInt16(1 + ofst, true);\nofst += 2\nvar f = (view1.getInt16(1 + ofst, true) / 100).toString();\nofst += 2\nelement.firstElementChild.style.cssText = \"transform: scale(\"+d+\".\"+f.split('.')[1]+\");\"\nelement.style.cssText = style;\n} catch (exp) {\n}\n}\n});\nreturn;\n}\nif (view[0] == 10) {\nvar offset = 1;\nvar inputThrottle = (view1.getFloat32(offset, true)*100).toFixed(2);\nvar inputThrottleGraph = view1.getFloat32(offset, true);\noffset += 4;\nvar inputBrake = (view1.getFloat32(offset, true)*100).toFixed(2);\nvar inputBrakeGraph = view1.getFloat32(offset, true);\noffset += 4;\nvar inputThrottleRaw = view1.getFloat32(offset, true)*100;\nvar inputThrottleRawGraph = view1.getFloat32(offset, true);\noffset += 4;\nvar inputBrakeRaw = view1.getFloat32(offset, true)*100;\noffset += 4;\nvar inputSteerRaw = (view1.getFloat32(offset, true) + 1) / 2;\noffset += 4;\nvar absFl = view1.getFloat32(offset, true);\noffset += 4;\nvar absFr = view1.getFloat32(offset, true);\noffset += 4;\nvar absRl = view1.getFloat32(offset, true);\noffset += 4;\nvar absRr = view1.getFloat32(offset, true);\noffset += 4;\nvar gripFl = view1.getFloat32(offset, true) * 100;\noffset += 4;\nvar gripFr = view1.getFloat32(offset, true) * 100;\noffset += 4;\nvar gripRl = view1.getFloat32(offset, true) * 100;\noffset += 4;\nvar gripRr = view1.getFloat32(offset, true) * 100;\n// inputsTds[0].innerHTML = absFl;\n// inputsTds[1].innerHTML = absFr;\n// inputsTds[2].innerHTML = absRl;\n// inputsTds[3].innerHTML = absRr;\ninputsTds[0].innerHTML = inputThrottle;\ninputsTds[1].innerHTML = inputBrake;\ninputsTds[2].innerHTML = inputThrottleRaw < 1 && inputThrottleRaw > 0 ? 1 : inputThrottleRaw.toFixed(1);\ninputsTds[3].innerHTML = inputBrakeRaw < 1 && inputBrakeRaw > 0 ? 1 : inputBrakeRaw.toFixed(1);\ndataAbs[0][0][0].push(absFl);\nif (dataAbs[0][0][0].length > maxPoints) {\ndataAbs[0][0][0].shift();\n}\ndataAbs[0][1][0].push(absFr);\nif (dataAbs[0][1][0].length > maxPoints) {\ndataAbs[0][1][0].shift();\n}\ndataAbs[1][0][0].push(absRl);\nif (dataAbs[1][0][0].length > maxPoints) {\ndataAbs[1][0][0].shift();\n}\ndataAbs[1][1][0].push(absRr);\nif (dataAbs[1][1][0].length > maxPoints) {\ndataAbs[1][1][0].shift();\n}\ndataAbs[0][0][1].push(gripFl);\nif (dataAbs[0][0][1].length > maxPoints) {\ndataAbs[0][0][1].shift();\n}\ndataAbs[0][1][1].push(gripFr);\nif (dataAbs[0][1][1].length > maxPoints) {\ndataAbs[0][1][1].shift();\n}\ndataAbs[1][0][1].push(gripRl);\nif (dataAbs[1][0][1].length > maxPoints) {\ndataAbs[1][0][1].shift();\n}\ndataAbs[1][1][1].push(gripRr);\nif (dataAbs[1][1][1].length > maxPoints) {\ndataAbs[1][1][1].shift();\n}\n// for (let i = 0; i < gridSize; i++) {\n//     for (let j = 0; j < gridSize; j++) {\n//         // Simulating new percentage data (You can replace this with your real data)\n//         const newPercentage = Math.random() * 100;\n//         data[i][j].push(newPercentage);\n//         // Remove the oldest data point if it exceeds the max points\n//         if (data[i][j].length > maxPoints) {\n//             data[i][j].shift();\n//         }\n//     }\n// }\ndata.labels.push(new Date().toLocaleTimeString());\ndata.datasets[0].data.push(inputBrakeGraph * canvas1.height);\ndata.datasets[1].data.push(inputThrottleGraph * canvas1.height);\ndata.datasets[2].data.push(inputThrottleRawGraph * canvas1.height);\ndata.datasets[3].data.push(inputSteerRaw * canvas1.height);\n// if (data.labels.length > canvas1.width / 1) {\nif (data.labels.length > 300) {\ndata.labels.shift();\ndata.datasets.forEach(dataset => dataset.data.shift());\n}\nif (absgripGraphSetts)\nanimate();\nif (inputGraphSetts)\ndrawChart();\nreturn;\n}\nif (view[0] == 11) {\nofst = 1;\nif ( view1.getInt8(ofst, true) == 0 ) {\nofst += 1;\nraceLaps.innerHTML = view1.getInt32(ofst, true);\nofst += 4;\nvar lastLapHtml = view1.getFloat32(ofst, true);\nofst += 4;\nvar res = Math.floor(lastLapHtml / 60);\nif ( res >= 1 ) {\nraceTime.innerHTML = (res+\"\").padStart(2, '0')+\":\"+(Math.floor(lastLapHtml - 60 * res)).toFixed(0).padStart(2, '0');\n} else {\nraceTime.innerHTML = sec.toFixed(0);\n}\nif ( flagStatusYellow != view1.getInt32(ofst, true) ) {\nflagStatusYellow = view1.getInt32(ofst, true)\nif ( view1.getInt32(ofst, true) == 1) {\nstandingsFlag.classList.remove('splitGreen');\nstandingsFlag.classList.add('splitYellow');\n} else {\nstandingsFlag.classList.remove('splitYellow');\nstandingsFlag.classList.add('splitGreen');\n}\n}\nofst += 4;\n} else {\nvar lel = view1.getInt8(ofst, true);\nofst += 1;\ntry {\nvar pos = view1.getInt32(ofst, true);\nofst += 4;\nvar drvrName = view1.getUTF16String(ofst, 64);\nlet parts = drvrName.split(' ');\nif (parts.length > 1) {\ndrvrName = parts[0].charAt(0) + '. ' + parts[1] + (parts[2]?' '+parts[2]:'');\n}\ndriverBlocks[pos].children[0].children[2].innerHTML = drvrName;\nofst += 64;\nif ( pos != 0 ) {\ndriverBlocks[pos].children[0].children[5].innerHTML = (view1.getFloat32(ofst, true) > 0 ?\"+\" : \"\") + view1.getFloat32(ofst, true).toFixed(3);\n} else {\nvar timeHtml = view1.getFloat32(ofst, true);\nif (timeHtml == -1) {\ndriverBlocks[pos].children[0].children[5].innerHTML = \"-:--.---\";\n} else {\nvar res = Math.floor(timeHtml / 60);\nif ( res >= 1 ) {\ndriverBlocks[pos].children[0].children[5].innerHTML = res+\":\"+(timeHtml - 60 * res).toFixed(3).padStart(6, '0');\n} else {\ndriverBlocks[pos].children[0].children[5].innerHTML = timeHtml.toFixed(3).padStart(6, '0');\n}\n}\n//driverBlocks[pos].children[0].children[5].innerHTML = \"+\" + view1.getFloat32(ofst, true).toFixed(3);\n}\nofst += 4;\ndriverBlocks[pos].children[0].children[4].innerHTML = view1.getUint16(ofst, true);\nofst += 2;\nif ( view1.getUint8(offset, true) ) {\ndriverBlocks[pos].children[0].children[3].classList.remove('vericalSeparatorRed');\ndriverBlocks[pos].children[0].children[3].classList.add('vericalSeparatorGreen');\n} else {\ndriver";
char* responz5 = "Blocks[pos].children[0].children[3].classList.remove('vericalSeparatorGreen');\ndriverBlocks[pos].children[0].children[3].classList.add('vericalSeparatorRed');\n}\nofst +=1;\n} catch (e) {console.log(e)}\n}\nreturn;\n}\nif (view[0] == 12) {\nvar table = document.getElementById(\"porscheTable\");\nvar lastChild = table.lastElementChild;\nvar driverBlock = document.getElementById(\"driverBlock\");\nvar iii = 1;\nfor ( var i = 0; i < view1.getInt32(1, true) - 1; i++ ) {\nlet copy = driverBlock.cloneNode(true);\ncopy.children[0].children[0].innerHTML = i + 2;\ncopy.style.setProperty('--i', i + 1);\n//table.appendChild(copy);\ntable.insertBefore(copy, lastChild);\niii = i + 1;\n}\ndocument.querySelector(\"#pImg\").style.setProperty('--i', i + 1);\ndriverBlocks = document.querySelectorAll(\".driverBlock\");\nreturn;\n}\nif (view[0] == 13) {\nvar ofst = 1;\nlastLapSetts = view1.getInt32(ofst, true);\nofst += 4;\nbestLapSetts = view1.getInt32(ofst, true);\nofst += 4;\nbestLapLeadSetts = view1.getInt32(ofst, true);\nofst += 4;\ndeltaSetts = view1.getInt32(ofst, true);\nofst += 4;\nradarSetts = view1.getInt32(ofst, true);\nofst += 4;\nallTimeBestTableSetts = view1.getInt32(ofst, true);\nofst += 4;\nrelativeTableSetts = view1.getInt32(ofst, true);\nofst += 4;\nwheelsSetts = view1.getInt32(ofst, true);\nofst += 4;\ninputGraphSetts = view1.getInt32(ofst, true);\nofst += 4;\nabsgripGraphSetts = view1.getInt32(ofst, true);\nofst += 4;\nslipGraphSetts = view1.getInt32(ofst, true);\nofst += 4;\ninputPercentsSetts = view1.getInt32(ofst, true);\nofst += 4;\nstandingsSetts = view1.getInt32(ofst, true);\nofst += 4;\nstartingLightsSetts = view1.getInt32(ofst, true);\nif ( !lastLapSetts )\ndocument.querySelector(\".lastlapHider\").classList.add('maxHide');\nif ( !bestLapSetts )\ndocument.querySelector(\".bestlapHider\").classList.add('maxHide');\nif ( !bestLapLeadSetts )\ndocument.querySelector(\".bestlapleadHider\").classList.add('maxHide');\nif ( !deltaSetts )\ndocument.querySelector(\".deltaHider\").classList.add('maxHide');\nif ( !radarSetts )\ndocument.querySelector(\".myCanvasHider\").classList.add('maxHide');\nif ( !allTimeBestTableSetts )\ndocument.querySelector(\".fuelmonitorHider\").classList.add('maxHide');\nif ( !relativeTableSetts )\ndocument.querySelector(\".relativeHider\").classList.add('maxHide');\nif ( !wheelsSetts )\ndocument.querySelector(\".wheelsHider\").classList.add('maxHide');\nif ( !absgripGraphSetts )\ndocument.querySelector(\".chartabsHider\").classList.add('maxHide');\nif ( !inputGraphSetts )\ndocument.querySelector(\".chartHider\").classList.add('maxHide');\nif ( !slipGraphSetts )\ncument.querySelector(\".chartSlipHider\").classList.add('maxHide');\nif ( !inputPercentsSetts )\ndocument.querySelector(\".inputsHider\").classList.add('maxHide');\nif ( !standingsSetts )\ndocument.querySelector(\".porscheTableHider\").classList.add('maxHide');\nif ( !startingLightsSetts )\ndocument.querySelector(\".startLightHider\").classList.add('maxHide');\nreturn;\n}\n} else {\nconsole.log(\"nonbin\");\nconsole.log(event.data);\n}\n});\nsocket.addEventListener('close', (event) => { console.log(`Server closed connection: ${event.code} ${event.reason}`); });\nsocket.addEventListener('error', (event) => { console.error('WebSocket error:', event); });\nsocket.addEventListener('onclose', (event) => { console.error('WebSocket closed:', event); });\n});\n</script>\n<style type=\"text/css\">\nbody {\nmargin: 0;\noverflow: hidden;\nuser-select: none;\n}\n.backgroundColorRed{\nbackground-color: #db5555;\n}\n.backgroundColorBlack{\nbackground-color: #000;\n}\n.backgroundColorYellow{\nbackground-color: #f7e372;\n}\n.backgroundColorGray{\nbackground-color: rgb(233, 232, 232, 0.3);\n}\n.hide {\ndisplay: none;\n}\n.maxHide {\ndisplay: none !important;\n}\n.border {\nborder: 1px solid #ccc;\n}\n.porscheTableImg {\ndisplay: flex;\nposition: relative;\npadding-top: 50px;\ntransform: translateY(calc(4px * var(--i)));\n}\n#resizable-table {\nposition: absolute;\nwidth: 1px;\nheight: 1px;\n}\ntable {\ntable-layout: fixed;\n}\nth,\ntd {\npadding: 0px;\nborder: 1px solid #ccc;\nwidth: 1px;\nwhite-space: nowrap;\n}\n#draggable-corner {\ndisplay: none;\nposition: absolute;\nwidth: 15px;\nheight: 15px;\ntop: 0;\nleft: 0;\nbackground-color: #3498db;\n}\n.tg {\nborder-collapse: collapse;\nborder-spacing: 0;\n}\n.tg td {\nborder-color: black;\nborder-style: solid;\nborder-width: 1px;\nfont-family: Arial, sans-serif;\nfont-size: 14px;\noverflow: hidden;\npadding: 10px 5px;\nword-break: normal;\n}\n.tg th {\nborder-color: black;\nborder-style: solid;\nborder-width: 1px;\nfont-family: Arial, sans-serif;\nfont-size: 14px;\nfont-weight: normal;\noverflow: hidden;\npadding: 10px 5px;\nword-break: normal;\n}\n.tg .tg-11kk {\nbackground-color: #656565;\nborder-color: #f8a102;\ncolor: #ffffff;\nfont-size: 36px;\ntext-align: center;\nvertical-align: middle\n}\n.tg .tg-agb7 {\nbackground-color: #fd6864;\nborder-color: #f8a102;\ncolor: #ffffff;\nfont-size: 36px;\ntext-align: center;\nvertical-align: middle\n}\n.tg .tg-2uwx {\nbackground-color: #343434;\nborder-color: #f8a102;\ncolor: #ffffff;\nfont-size: 16px;\nfont-weight: bold;\ntext-align: center;\nvertical-align: middle\n}\n.tg .tg-6wkr {\nbackground-color: #656565;\nborder-color: #f8a102;\ncolor: #fd6864;\nfont-size: 18px;\nfont-weight: bold;\ntext-align: center;\nvertical-align: middle\n}\n.tg .tg-vr5q {\nbackground-color: #656565;\nborder-color: #f8a102;\ncolor: #67fd9a;\nfont-size: 18px;\nfont-weight: bold;\ntext-align: center;\nvertical-align: middle\n}\n.tg .tg-ivzj {\nbackground-color: #656565;\nborder-color: #9b9b9b;\ncolor: #ffffff;\ntext-align: center;\nvertical-align: middle\n}\n.tg .tg-a0w5 {\nbackground-color: #656565;\ncolor: #ffffff;\ntext-align: center;\nvertical-align: middle\n}\n.tg .tg-be88 {\nbackground-color: #656565;\nborder-color: inherit;\ncolor: #ffffff;\nfont-size: 36px;\ntext-align: center;\nvertical-align: middle\n}\n.tg .tg-wqw7 {\nbackground-color: #656565;\nborder-color: inherit;\ncolor: #ffffff;\nfont-size: 24px;\ntext-align: center;\nvertical-align: middle\n}\n.tg .tg-yeut {\nbackground-color: #656565;\nborder-color: inherit;\ncolor: #ffffff;\nfont-weight: bold;\ntext-align: center;\nvertical-align: middle\n}\n.tg .tg-a0w51{\nbackground-color:#656565;\ncolor:#ffffff;\ntext-align:center;\nvertical-align:middle;\nheight: 5px;\n}\n.tg .tg-xtf7{background-color:#656565;border-color:#000000;color:#ffffff;font-weight:bold;text-align:center;vertical-align:middle}\n.tg .tg-11ac{background-color:#656565;border-color:#000000;color:#ffffff;font-size:28px;font-weight:bold;text-align:center;\nvertical-align:middle}\n.outer-block {\ndisplay: flex;\nflex-direction: column;\njustify-content: center;\nalign-items: center;\nfont-size: 26px;\nfont-weight: bold;\ncolor: white;\ntext-shadow: -1px 0 black, 0 1px black, 1px 0 black, 0 -1px black;\n}\n.scaler {\nwhite-space: nowrap;\ndisplay: flex;\nflex-direction: column;\njustify-content: center;\nalign-items: center;\nfont-size: 26px;\nfont-weight: bold;\ncolor: white;\ntext-shadow: -1px 0 black, 0 1px black, 1px 0 black, 0 -1px black;\n}\n#toggle-settings {\nposition: absolute;\nwidth: 15px;\nheight: 15px;\ntop: 0;\nright: 0;\nbackground-color: rgba(255, 255, 255, 0);\n}\n#reload-page {\nposition: absolute;\nwidth: 5px;\nheight: 5px;\ntop: 0;\nleft: 0;\nbackground-color: rgba(255, 255, 255, 0);\n}\n.over {\ndisplay: grid;\ngrid-template-columns: repeat(2, 1fr);\ngrid-template-rows: repeat(2, 1fr);\njustify-items: center;\nalign-items: center;\nposition: relative;\ngrid-row-gap: 7px;\ngrid-column-gap: 5px;\n}\n.container {\ndisplay: flex;\njustify-content: center;\nalign-items: center;\nheight: 300px;\nposition: relative;\n}\n.div {\nwidth: 100px;\nheight: 300px;\nbackground-color: blue;\nborder: 2px solid black;\n}\n.div:first-child {\nborder-top-left-radius: 10px;\nborder-bottom-left-radius: 10px;\n}\n.div:last-child {\nborder-top-right-radius: 10px;\nborder-bottom-right-radius: 10px;\n}\n.content {\nposition: relative;\ntop: 10px;\nleft: calc(23%);\ncolor: white;\nfont-size: 40px;\ntext-shadow: -2px 1px black, 0 2px black, 2px 0 black, 0 -2px black;\n}\n.data {\ndisplay: inline;\n}\n.sign {\ndisplay: inline;\n}\n.big-text {\nposition: absolute;\ntop: calc(56%);\nleft: calc(50%);\ncolor: white;\nfont-size: 90px;\ntransform: translate(-50%, -50%);\ntext-shadow: -2px 0 black, 0 2px black, 2px 0 black, 0 -2px black;\n}\n.upper-text {\ntext-wrap: nowrap;\nposition: absolute;\ntop: calc(24%);\nleft: calc(50%);\ncolor: white;\nfont-size: 35px;\ntransform: translateX(-50%);\ntext-shadow: -2px 0 black, 0 2px black, 2px 0 black, 0 -2px black;\n}\n.under-text {\nposition: absolute;\ntop: calc(77%);\nleft: calc(50%);\ncolor: white;\nfont-size: 40px;\ntransform: translateX(-50%);\ntext-shadow: -2px 0 black, 0 2px black, 2px 0 black, 0 -2px black;\n}\n.small-div {\nwidth: 30px;\nheight: 100px;\nbackground-color: blue;\nborder-radius: 10px;\nborder: 2px solid black;\nposition:relative;\nleft:-38px;\nbottom: -60px;\n}\n.small-div-right {\nwidth: 30px;\nheight: 100px;\nbackground-color: blue;\nborder-radius: 10px;\nborder: 2px solid black;\nposition:relative;\nleft: 312px;\nbottom: -60px;\n}\n.vertical-text {\nposition: absolute;\ncolor: white;\nfont-size: x-large;\ntransform-origin: center center;\ntop: 50%;\nleft: calc(50% - 2px);\nwidth: 70px;\ntransform: translate(-50%, -50%) rotate(90deg);\ntext-align: center;\n}\n.traffic-light {\nwidth: 50px;\nbackground-color: #333;\nborder-radius: 10px;\npadding: 10px;\n}\n.light {\nheight: 50px;\nwidth: 50px;\nmargin-bottom: 10px;\nborder-radius: 50%;\n}\n.red {\nbackground-color: #f00;\n}\n.green {\nbackground-color: #0f0;\n}\n.off {\nbackground-color: #555;\n}\n.graphbackground {\nbackground-color: rgba(2, 2, 2, 0.5);\n}\n@font-face {\nfont-family: 'porsche-next';\nsrc: url('http://localhost:8081/porsche-next.otf');\n}\n@font-face {\nfont-family: 'porsche-next-semibold';\nsrc: url('http://localhost:8081/porsche-next-semibold.otf');\n}\n:root {\n--widthLap: 50px;\n--widthTime: 220px;\n--heightHeader: 50px;\n--widthTable: calc(var(--widthLap) + var(--widthTime));\n--heightHeaderSeparator: 10px;\n--widthVerticalSeparator: 4px;\n--heightDriverLine: 50px;\n--widthPit: 50px;\n--widthDriverTime: 100px;\n--heightRepSeparator: 4px;\n--widthRep: 100px;\n--widthTriangleRep: 10px;\n--heightRep: 15px;\n}\n.header {\ndisplay: flex;\ncolor: #fff;\n}\n.part {\ndisplay: flex;\n}\n.text {\nfont-size: 20px;\nfont-weight: normal;\nfont-family: 'porsche-next', Fallback, sans-serif;\ndisplay: flex;\nalign-items: center;\njustify-content: center;\n}\n.textRep {\nfont-size: 11px;\nfont-weight: normal;\nfont-family: 'porsche-next', Fallback, sans-serif;\ndisplay: flex;\nalign-items: center;\njustify-content: center;\n}\n.textSemibold {\nfont-size: 20px;\nfont-weight: normal;\nfont-family: 'porsche-next-semibold', Fallback, sans-serif;\ndisplay: flex;\nalign-items: center;\njustify-content: center;\n}\n.topRectangleFirst {\nwidth: var(--widthLap);\nheight: var(--heightHeader);\nbackground-color: #000;\nborder-top-left-radius: 20px;\n}\n.topRectangleSecond {\nwidth: var(--widthTime);\nheight: var(--heightHeader);\nbackground-color: #000;\n}\n.rectangularBarRep {\nwidth: var(--widthRep);\nheight: var(--heightRep);\nbackground-color: #000;\ncolor: #fff;\n}\n.triangleRep {\nmargin-left: calc(var(--widthTable) - var(--widthTriangleRep) - var(--widthRep));\nwidth: 0;\nheight: 0;\nborder-left: var(--widthTriangleRep) solid transparent;\nborder-right: 0px solid transparent;\nborder-top: var(--heightRep) solid black;\n}\n.splitBlack {\nwidth: calc(var(--widthTable));\nheight: var(--heightHeaderSeparator);\nbackground-color: #000;\n}\n.splitGreen {\nwidth: calc(var(--widthTable));\nheight: var(--heightHeaderSeparator);\nbackground-color: rgb(6, 250, 12);\n}\n.splitYellow {\nwidth: calc(var(--widthTable));\nheight: var(--heightHeaderSeparator);\nbackground-color: #f8fb02;\n}\n.driverLine {\ndisplay: flex;\ncolor: #fff;\n}\n.positionRect {\nwidth: var(--widthLap);\nheight: var(--heightDriverLine);\nbackground-color: #000;\n}\n.v";
char* responz6 = "ericalSeparatorAlpha {\nwidth: var(--widthVerticalSeparator);\nheight: var(--heightDriverLine);\nbackground-color: rgba(0, 0, 0, 0);\n}\n.vericalSeparatorRed {\nwidth: var(--widthVerticalSeparator);\nheight: var(--heightDriverLine);\nbackground-color: rgb(255, 49, 29);\n}\n.vericalSeparatorGreen {\nwidth: var(--widthVerticalSeparator);\nheight: var(--heightDriverLine);\nbackground-color: rgb(49, 255, 29);\n}\n.driverName {\nwidth: calc(var(--widthTable) - var(--widthVerticalSeparator) * 2 - var(--widthLap) - var(--widthPit));\nheight: var(--heightDriverLine);\nbackground-color: #000;\n}\n.pit {\nwidth: var(--widthPit);\nheight: var(--heightDriverLine);\nbackground-color: #fff;\ncolor: #000;\n}\n.driverTimeFirst {\nwidth: var(--widthDriverTime);\nheight: var(--heightDriverLine);\nbackground-color: #000;\ncolor: #fff;\nborder-bottom-right-radius: 20px;\n}\n.driverTime {\nwidth: var(--widthDriverTime);\nheight: var(--heightDriverLine);\nbackground-color: rgba(233, 232, 232, 0.3);\ncolor: #fff;\nborder-bottom-right-radius: 20px;\n}\n.scaler .driverSeparatorAbove {\nwidth: var(--widthDriverTime);\nheight: var(--heightRepSeparator);\nbackground-color: #fff0;\ncolor: #000;\n}\n.driverSeparatorUnder {\nwidth: calc(var(--widthTable));\nheight: var(--heightRepSeparator);\nbackground-color: #fff0;\ncolor: #000;text-align: right;\n}\n.scaler .driverBlock {\ndisplay: flex;\nflex-direction: row-reverse;\nflex-wrap: nowrap;\ntransform: translateY(calc(4px * var(--i)));\n}\n.flexingParent {\n/* display: inline-block; */\ndisplay: flex;\n}\n.flexingChild {\ndisplay: grid;\njustify-items: center;\n}\n.rectangle {\nposition: relative;\nborder-top-left-radius: 35px;\nborder-bottom-left-radius: 5px;\nwidth: 112px;\nheight: 50px;\nbackground-color: white;\ndisplay: inline-block;\n}\n.triangle {\nposition: relative;\nwidth: 0;\nheight: 0;\nborder-right: 25px solid transparent;\nborder-top: 50px solid white;\ndisplay: inline-block;\n}\n.rectangle1 {\nposition: relative;\nleft: -18px;\nborder-top-right-radius: 5px;\nborder-bottom-right-radius: 35px;\nwidth: 66px;\nheight: 50px;\nbackground-color: white;\ndisplay: inline-block;\n}\n.triangle1 {\nposition: relative;\nleft: -18px;\nwidth: 0;\nheight: 0;\nborder-left: 25px solid transparent;\nborder-bottom: 50px solid white;\ndisplay: inline-block;\n}\n.textBlockTime {\nposition: absolute;\ntop: 50%;\nleft: 50%;\ntransform: translate(-38%, -50%);\ncolor: black;\nfont-weight: bold;\nfont-size: 20px;\n}\n.textBlockDelta {\nposition: absolute;\ntop: 50%;\nleft: 50%;\ntransform: translate(-62%, -50%);\ncolor: red;\nfont-weight: bold;\nfont-size: 20px;\n}\n</style>\n</head>\n<body class=\"hide\">\n<div id=\"resizable-table\" class=\"outer-block fuelmonitorHider\">\n<div class=\"scaler\">\n<table class=\"tg\" id=\"fuelmonitor\">\n<tbody>\n<tr>\n<td class=\"tg-xtf7\">Fuel/Lap</td>\n<td class=\"tg-xtf7\">Best Lap</td>\n</tr>\n<tr>\n<td class=\"tg-11ac\">9.99</td>\n<td class=\"tg-11ac\">8:32.000</td>\n</tr>\n<tr>\n<td class=\"tg-11ac\">432.00</td>\n<td class=\"tg-11ac\">200.00</td>\n</tr>\n<tr>\n<td class=\"tg-xtf7\">Fuel Race</td>\n<td class=\"tg-xtf7\">Fuel Remain</td>\n</tr>\n</tbody>\n</table>\n</div>\n<div id=\"draggable-corner\" class=\"hide\"></div>\n</div>\n<div id=\"resizable-table\" class=\"outer-block relativeHider\">\n<div class=\"scaler\">\n<table class=\"tg\" id=\"relative\">\n<tbody>\n<tr>\n<td class=\"tg-a0w5\">0</td>\n<td class=\"tg-a0w5\">Loading...</td>\n<td class=\"tg-a0w5\">0/0</td>\n<td class=\"tg-a0w5\">0</td>\n</tr>\n<tr>\n<td class=\"tg-a0w5\">0</td>\n<td class=\"tg-a0w5\">Loading...</td>\n<td class=\"tg-a0w5\">0/0</td>\n<td class=\"tg-a0w5\">0</td>\n</tr>\n<tr>\n<td class=\"tg-a0w5\">0</td>\n<td class=\"tg-a0w5\">Loading...</td>\n<td class=\"tg-a0w5\">0/0</td>\n<td class=\"tg-a0w5\">0</td>\n</tr>\n<tr>\n<td class=\"tg-a0w5\">0</td>\n<td class=\"tg-a0w5\">Loading...</td>\n<td class=\"tg-a0w5\">0/0</td>\n<td class=\"tg-a0w5\">0</td>\n</tr>\n<tr>\n<td class=\"tg-a0w5\">0</td>\n<td class=\"tg-a0w5\">Loading...</td>\n<td class=\"tg-a0w5\">0/0</td>\n<td class=\"tg-a0w5\">0</td>\n</tr>\n<tr>\n<td class=\"tg-a0w5\">0</td>\n<td class=\"tg-a0w5\" style=\"background-color: rgb(233, 232, 232);\">Loading...</td>\n<td class=\"tg-a0w5\">0/0</td>\n<td class=\"tg-a0w5\">0</td>\n</tr>\n<tr>\n<td class=\"tg-a0w5\">0</td>\n<td class=\"tg-a0w5\">Loading...</td>\n<td class=\"tg-a0w5\">0/0</td>\n<td class=\"tg-a0w5\">0</td>\n</tr>\n<tr>\n<td class=\"tg-a0w5\">0</td>\n<td class=\"tg-a0w5\">Loading...</td>\n<td class=\"tg-a0w5\">0/0</td>\n<td class=\"tg-a0w5\">0</td>\n</tr>\n<tr>\n<td class=\"tg-a0w5\">0</td>\n<td class=\"tg-a0w5\">Loading...</td>\n<td class=\"tg-a0w5\">0/0</td>\n<td class=\"tg-a0w5\">0</td>\n</tr>\n<tr>\n<td class=\"tg-a0w5\">0</td>\n<td class=\"tg-a0w5\">Loading...</td>\n<td class=\"tg-a0w5\">0/0</td>\n<td class=\"tg-a0w5\">0</td>\n</tr>\n<tr>\n<td class=\"tg-a0w5\">0</td>\n<td class=\"tg-a0w5\">Loading...</td>\n<td class=\"tg-a0w5\">0/0</td>\n<td class=\"tg-a0w5\">0</td>\n</tr>\n</tbody>\n</table>\n</div>\n<div id=\"draggable-corner\" class=\"hide\"></div>\n</div>\n<div id=\"resizable-table\" class=\"canv myCanvasHider\">\n<div class=\"scaler\">\n<canvas class=\"border\" id=\"myCanvas\" width=\"400\" height=\"400\"></canvas>\n</div>\n<div id=\"draggable-corner\" class=\"hide\"></div>\n</div>\n<div id=\"resizable-table\" class=\"outer-block deltaHider\">\n<div class=\"scaler\">\n<div id=\"delta\" class=\"inner-block\">-</div>\n<div class=\"inner-block\">Delta</div>\n</div>\n<div id=\"draggable-corner\" class=\"hide\"></div>\n</div>\n<div id=\"resizable-table\" class=\"outer-block bestlapHider\">\n<div class=\"scaler\">\n<div id=\"bestlap\" class=\"inner-block\">-</div>\n<div class=\"inner-block\">Best Lap</div>\n</div>\n<div id=\"draggable-corner\" class=\"hide\"></div>\n</div>\n<div id=\"resizable-table\" class=\"outer-block bestlapleadHider\">\n<div class=\"scaler\">\n<div id=\"bestlaplead\" class=\"inner-block\">-</div>\n<div class=\"inner-block\">Best Lap Leader</div>\n</div>\n<div id=\"draggable-corner\" class=\"hide\"></div>\n</div>\n<div id=\"resizable-table\" class=\"outer-block lastlapHider\">\n<div class=\"scaler\">\n<div id=\"lastlap\" class=\"inner-block\">-</div>\n<div class=\"inner-block\">Last Lap</div>\n</div>\n<div id=\"draggable-corner\" class=\"hide\"></div>\n</div>\n<div id=\"resizable-table\" class=\"outer-block inputsHider\">\n<div class=\"scaler\">\n<table class=\"tg\" id=\"inputs\">\n<tbody>\n<tr>\n<td class=\"tg-a0w51\">100</td>\n<td class=\"tg-a0w51\">100</td>\n</tr>\n<tr>\n<td class=\"tg-a0w51\">100</td>\n<td class=\"tg-a0w51\">100</td>\n</tr>\n</tbody>\n</table>\n</div>\n<div id=\"draggable-corner\" class=\"hide\"></div>\n</div>\n<div id=\"toggle-settings\"></div>\n<div id=\"reload-page\"></div>\n<div id=\"resizable-table\" class=\"outer-block wheelsHider\">\n<div class=\"over scaler\">\n<div class=\"container\" id=\"wheels\">\n<div class=\"div\">\n<div class=\"content\"><div class=\"data\" id=\"flTemp1\">123</div><div class=\"sign\">°</div></div>\n<div class=\"big-text\"><div class=\"data\" id=\"flWear\">100</div><div class=\"sign\"></div></div>\n<div class=\"upper-text\"><div class=\"data\" id=\"flPressure\">241</div><div class=\"sign\"> kPa</div></div>\n<div class=\"under-text\"><div class=\"data\" id=\"flDirt\">100</div><div class=\"sign\">%</div></div>\n<div class=\"small-div\"><div class=\"vertical-text\"><div class=\"data\" id=\"flBrake\">123</div><div class=\"sign\">°</div></div></div>\n</div>\n<div class=\"div\">\n<div class=\"content\"><div class=\"data\" id=\"flTemp2\">456</div><div class=\"sign\">°</div></div>\n</div>\n<div class=\"div\">\n<div class=\"content\"><div class=\"data\" id=\"flTemp3\">789</div><div class=\"sign\">°</div></div>\n</div>\n</div>\n<div class=\"container\">\n<div class=\"div\">\n<div class=\"content\"><div class=\"data\" id=\"frTemp1\">123</div><div class=\"sign\">°</div></div>\n<div class=\"big-text\"><div class=\"data\" id=\"frWear\">100</div><div class=\"sign\"></div></div>\n<div class=\"upper-text\"><div class=\"data\" id=\"frPressure\">241</div><div class=\"sign\"> kPa</div></div>\n<div class=\"under-text\"><div class=\"data\" id=\"frDirt\">100</div><div class=\"sign\">%</div></div>\n<div class=\"small-div-right\"><div class=\"vertical-text\"><div class=\"data\" id=\"frBrake\">123</div><div class=\"sign\">°</div></div></div>\n</div>\n<div class=\"div\">\n<div class=\"content\"><div class=\"data\" id=\"frTemp2\">456</div><div class=\"sign\">°</div></div>\n</div>\n<div class=\"div\">\n<div class=\"content\"><div class=\"data\" id=\"frTemp3\">789</div><div class=\"sign\">°</div></div>\n</div>\n</div>\n<div class=\"container\">\n<div class=\"div\">\n<div class=\"content\"><div class=\"data\" id=\"rlTemp1\">123</div><div class=\"sign\">°</div></div>\n<div class=\"big-text\"><div class=\"data\" id=\"rlWear\">100</div><div class=\"sign\"></div></div>\n<div class=\"upper-text\"><div class=\"data\" id=\"rlPressure\">241</div><div class=\"sign\"> kPa</div></div>\n<div class=\"under-text\"><div class=\"data\" id=\"rlDirt\">100</div><div class=\"sign\">%</div></div>\n<div class=\"small-div\"><div class=\"vertical-text\"><div class=\"data\" id=\"rlBrake\">123</div><div class=\"sign\">°</div></div></div>\n</div>\n<div class=\"div\">\n<div class=\"content\"><div class=\"data\" id=\"rlTemp2\">456</div><div class=\"sign\">°</div></div>\n</div>\n<div class=\"div\">\n<div class=\"content\"><div class=\"data\" id=\"rlTemp3\">789</div><div class=\"sign\">°</div></div>\n</div>\n</div>\n<div class=\"container\">\n<div class=\"div\">\n<div class=\"content\"><div class=\"data\" id=\"rrTemp1\">123</div><div class=\"sign\">°</div></div>\n<div class=\"big-text\"><div class=\"data\" id=\"rrWear\">100</div><div class=\"sign\"></div></div>\n<div class=\"upper-text\"><div class=\"data\" id=\"rrPressure\">241</div><div class=\"sign\"> kPa</div></div>\n<div class=\"under-text\"><div class=\"data\" id=\"rrDirt\">111</div><div class=\"sign\">%</div></div>\n<div class=\"small-div-right\"><div class=\"vertical-text\"><div class=\"data\" id=\"rrBrake\">123</div><div class=\"sign\">°</div></div></div>\n</div>\n<div class=\"div\">\n<div class=\"content\"><div class=\"data\" id=\"rrTemp2\">456</div><div class=\"sign\">°</div></div>\n</div>\n<div class=\"div\">\n<div class=\"content\"><div class=\"data\" id=\"rrTemp3\">789</div><div class=\"sign\">°</div></div>\n</div>\n</div>\n</div>\n<div id=\"draggable-corner\" class=\"hide\"></div>\n</div>\n<div id=\"resizable-table\" class=\"outer-block startLightHider\">\n<div class=\"scaler\">\n<div id=\"startLight\" class=\"inner-block traffic-light hide\">\n<div class=\"light off\"></div>\n<div class=\"light off\"></div>\n<div class=\"light off\"></div>\n<div class=\"light off\"></div>\n<div class=\"light off\"></div>\n<div class=\"light off\"></div>\n</div>\n</div>\n<div id=\"draggable-corner\" class=\"hide\"></div>\n</div>\n<div id=\"resizable-table\" class=\"chartabsHider\">\n<div class=\"scaler\">\n<div class=\"graphbackground\">\n<canvas id=\"graphCanvas\" width=\"600\" height=\"150\"></canvas>\n</div>\n</div>\n<div id=\"draggable-corner\" class=\"hide\"></div>\n</div>\n<div id=\"resizable-table\" class=\"chartHider\">\n<div class=\"scaler\">\n<div class=\"graphbackground\">\n<canvas id=\"chart\" width=\"400\" height=\"150\"></canvas>\n</div>\n</div>\n<div id=\"draggable-corner\" class=\"hide\"></div>\n</div>\n<div id=\"resizable-table\" class=\"chartSlipHider\">\n<div class=\"scaler\">\n<div class=\"graphbackground\">\n<canvas id=\"chartSlip\" width=\"400\" height=\"150\"></canvas>\n</div>\n</div>\n<div id=\"draggable-corner\" class=\"hide\"></div>\n</div>\n<div id=\"resizable-table\" class=\"porscheTableHider flexingParent\">\n<div id=\"porscheTable\" class=\"scaler flexingChild\">\n<div class=\"header\"><div id=\"raceLaps\" class=\"topRectangleFirst text\">66</div><div id=\"raceTime\" class=\"topRectangleSecond text\">99:59:59</div></div>\n<div class=\"splitBlack\"></div>\n<div id=\"splitFlag\" class=\"splitGreen\"></div>\n<div class=\"driverBlock\">\n<div class=\"driverLine\"><d";
char* responz7 = "iv class=\"positionRect text\">1</div><div class=\"vericalSeparatorAlpha\"></div><div class=\"driverName text\">A. RandomName</div><div class=\"vericalSeparatorRed\"></div><div class=\"pit text\">435</div><div class=\"driverTimeFirst textSemibold\">Gap</div></div>\n<div class=\"driverSeparatorAbove\"></div>\n</div>\n<img src=\"http://localhost:8081/2.png\" alt=\"Description of the image\" class=\"porscheTableImg\" id=\"pImg\">\n</div>\n<div class=\"hide\"><div id=\"driverBlock\" class=\"driverBlock\">\n<div class=\"driverLine\"><div class=\"positionRect text\">2</div><div class=\"vericalSeparatorAlpha\"></div><div class=\"driverName text\">A. RandomName</div><div class=\"vericalSeparatorRed\"></div><div class=\"pit text\"></div><div class=\"driverTime textSemibold\">99.999</div></div>\n<div class=\"driverSeparatorAbove\"></div>\n</div></div>\n<div id=\"draggable-corner\" class=\"hide\"></div>\n</div>\n</body>\n</html>\n";
char* responz;
char* html;

BOOL isClientConnected = FALSE;
BOOL isClientNew = FALSE;
SOCKET clientSocketW;

DWORD WINAPI thread_function(LPVOID lpParam) {
    FILE* file;

    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    SOCKADDR_IN serverAddr, clientAddr;
    char buffer[1024];
    int iResult, clientAddrLen = sizeof(clientAddr);

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    // Create a socket
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        printf("socket failed: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Bind the socket to an IP address and port
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(settings.http_port));
    //serverAddr.sin_port = htons(80);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    iResult = bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    if (iResult == SOCKET_ERROR) {
        printf("bind failed: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    iResult = listen(serverSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("Server is listening on port %s...\n", settings.http_port);

    while (1) {
        // Accept a client connection request
        //printf("\nready to accccept\n");
        clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) {
            printf("accept failed: %d\n", WSAGetLastError());
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }
        //printf("accccepted\n");
        struct timeval timeout;
        timeout.tv_sec = 1; // Timeout in seconds
        timeout.tv_usec = 500000;

        if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == SOCKET_ERROR) {
            printf("setsockopt failed: %d\n", WSAGetLastError());
        }
        while (1) {
            // Receive data from the client
            if (clientSocket == INVALID_SOCKET) {
                printf("Accept failed: %d\n", WSAGetLastError());
            }
            //else {
            //    // Get client IP address and port
            //    char clientIP[100];
            //    inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
            //    printf("Client connected from IP: %s, Port: %d\n", clientIP, ntohs(clientAddr.sin_port));
            //}
            memset(buffer, 0, sizeof(buffer));
            iResult = recv(clientSocket, buffer, sizeof(buffer), 0);

            //printf("%d\n", iResult);
            if (iResult > 0) {
                //printf("%s", buffer);
                char method[16], path[256], protocol[16];
                //sscanf(buffer, "%s %s %s", method, path, protocol);
                sscanf_s(buffer, "%s %s %s", method, (unsigned)_countof(method), path, (unsigned)_countof(path), protocol, (unsigned)_countof(protocol));
                if (strlen(path + 1) > 0) {
                    //if (strcmp(str1, "favicon.ico"))
                    errno_t err = fopen_s(&file, path + 1, "rb");
                    //printf("%d ", strlen(path + 1));
                    //printf("%s", path + 1);
                    //file = fopen(path + 1, "r");  // +1 to skip the leading '/'
                    if (err == 0) {

                        
                        fseek(file, 0, SEEK_END);
                        long file_size = ftell(file);
                        rewind(file);

                        char* file_buffer = malloc(file_size);
                        if (file_buffer == NULL) {
                            // Memory allocation failed, send 500
                            char* headers = "HTTP/1.1 500 Internal Server Error\r\n\r\nMemory allocation failed";
                            send(clientSocket, headers, strlen(headers), 0);
                        } else {
                            // Read the file into the buffer
                            fread(file_buffer, 1, file_size, file);

                            // Send the headers
                            char* headers = "HTTP/1.1 200 OK\r\nContent-Type: font/otf\r\n\r\n";
                            send(clientSocket, headers, strlen(headers), 0);

                            // Send the file
                            send(clientSocket, file_buffer, file_size, 0);

                            // Free the buffer
                            free(file_buffer);
                        }

                        // File found, send it
                        //char* headers = "HTTP/1.1 200 OK\r\nContent-Type: font/otf\r\n\r\n";
                        //send(clientSocket, headers, strlen(headers), 0);
                        /*/char file_buffer[2048];
                        int file_size;
                        while ((file_size = fread(file_buffer, 1, 2048, file)) > 0) {
                            send(clientSocket, file_buffer, file_size, 0);
                        }
                        fclose(file);*/
                    }
                    else {
                        // File not found, send 404
                        char* message = "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
                        send(clientSocket, message, strlen(message), 0);
                    }
                }
                else {
                    if (settings.dev == 1) {
                        checkIsHtmlsAreExistsAndCreateThemIfNotOrFillTheStringsWithIt();
                    }
                    send(clientSocket, responz, strlen(responz), 0);
                    isClientConnected = FALSE;
                    closesocket(clientSocketW);
                    clientSocketW = NULL;
                }
                //send(clientSocket, responz, strlen(responz), 0);
                
                closesocket(clientSocket);
                break;
                //printf("Sent data to client: %s\n", response);
                //Sleep(1000);
                //printf("Connection closed.\n");

            }
            else if (iResult == 0) {
                closesocket(clientSocket);
                break;
                //printf("Connection closed.\n");
            }
            else {
                //printf("recv failed: %d\n", WSAGetLastError());
                closesocket(clientSocket);
                //WSACleanup();
                break;
                //return 1;
            }

            // Close the client socket
            closesocket(clientSocket);
        }
    }
    // Close the server socket and cleanup Winsock
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}

int lenOfWinSizes = 0;
BYTE dataOfWinSizes[200];
DWORD WINAPI thread_function1(LPVOID lpParam) {
    WSADATA wsaData;
    SOCKET serverSocket;
    SOCKADDR_IN serverAddr, clientAddr;
    char buffer[1024];
    int iResult, clientAddrLen = sizeof(clientAddr);

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    // Create a socket
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        printf("socket failed: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Bind the socket to an IP address and port
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(settings.wesocket_port));
    //serverAddr.sin_port = htons(80);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    iResult = bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    if (iResult == SOCKET_ERROR) {
        printf("bind failed: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    // Listen for incoming connections
    iResult = listen(serverSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    printf("Server is listening on port %s...\n", settings.wesocket_port);
    while (1) {
        // Accept a client connection request
        clientSocketW = accept(serverSocket, (SOCKADDR*)&clientAddr, &clientAddrLen);
        if (clientSocketW == INVALID_SOCKET) {
            printf("accept failed: %d\n", WSAGetLastError());
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }
        // Receive data from the client
        memset(buffer, 0, sizeof(buffer));
        iResult = recv(clientSocketW, buffer, sizeof(buffer), 0);
        if (iResult > 0) {
            char code[25];
            int count = 0;
            for (int i = 0; i < sizeof(buffer); i++) {
                if (buffer[i] == '\n') {
                    count++;
                    if (count == 12) {
                        for (int j = 0, k = i - 25; j < 25; j++, k++) {
                            code[j] = buffer[k];
                        }
                        code[24] = '\0';
                        //printf("%s\n", code);
                        break;
                    }
                }
            }
            //printf("%s\n", code);
            char* charsss = gethandshakeKey(code);
            char charz[30];
            strcpy_s(charz, 30, charsss);
            //printf("%s\n", charz);
            //printf("Received data from client: %s\n", buffer);

            char response[200] = "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=\r\n\r\n";
            for (int i = 97, k = 0; i < 124; i++, k++) {
                response[i] = charz[k];
            }
            /*response[125] = '\r';
            response[126] = '\n';
            response[127] = '\r';
            response[128] = '\n';*/
            //printf("%s\n", response);
            send(clientSocketW, response, strlen(response), 0);
            //iResult = recv(clientSocket, buffer, sizeof(buffer), 0);
            unsigned char chrs[200];
            chrs[0] = 130;

            chrs[2] = 13;
            
            size_t remainingSize = sizeof(Settings) - (sizeof(settings.http_port) + sizeof(settings.wesocket_port));
            memcpy(chrs + 3, &settings.last_lap, remainingSize);
            chrs[1] = remainingSize + 1;

            //memcpy(chrs + 3, &settings, sizeof(Settings));
            //chrs[1] = sizeof(Settings) + 1;
            send(clientSocketW, chrs, chrs[1] + 2, 0);

            chrs[2] = 9;
            //BlobResult resp = readWindowsSettings();
            BlobResult resp = { 0, (unsigned char*)calloc(200 * sizeof(unsigned char), 1) };
            readWindowsSettings(&resp);

            //for (int i = 0; i < 112; i++) {
            //    printf("%d: %d\n", i, resp.data[i]);  // Print as integer
            //}
            if (resp.size < 10) {
                chrs[1] = 1 + 1;
                send(clientSocketW, chrs, chrs[1] + 2, 0);
                goto fldata;
            }
            //printf("\n");
            //printf("Values in decimal:\n");
            ///*printf("0: 9\n");
            //printf("1: 0\n");*/
            //for (int i = 0; i < 104; i++) {
            //    printf("%d: %d\n", i, (signed char)resp.data[i]);  // Print as integer
            //}
            //printf("\n");
            //printf("%d", resp.size);
            //for (int i = 0; i < resp.size; i++) {
            //    printf("%d: %d\n", i, resp.data[i]);  // Print as integer
            //}
            
            

            chrs[2] = 9;
            chrs[1] = 96 + 1;
            memcpy(chrs + 3, resp.data, 96);

            send(clientSocketW, chrs, chrs[1] + 2, 0);

            chrs[2] = 99;
            chrs[1] = resp.size - 96 + 1;
            memcpy(chrs + 3, resp.data + 96, resp.size - 96);
            send(clientSocketW, chrs, chrs[1] + 2, 0);
            free(resp.data);


            //BlobResult part1, part2;
            //part1.size = 98;
            //part1.data = (unsigned char*)malloc(part1.size);
            ////part1.data = (unsigned char*)calloc(part1.size, sizeof(unsigned char));
            ////memset(part1.data, 0, part1.size);
            //memmove(part1.data, resp.data, part1.size);

            //printf("a");
            //for (int i = 0; i < resp.size; i++) {
            //    printf("%d: %d\n", i, resp.data[i]);  // Print as integer
            //}
            //for (int i = 0; i < part1.size; i++) {
            //    printf("%d: %d\n", i, part1.data[i]);  // Print as integer
            //}
            ////for (int i = 0; i < resp.size; i++) {
            ////    printf("%d: %d\n", i, resp.data[i]);  // Print as integer
            ////}

            //part2.size = resp.size - 98;
            //part2.data = (unsigned char*)malloc(part2.size);
            ////part2.data = (unsigned char*)calloc(part2.size, sizeof(unsigned char));
            //memcpy(part2.data, resp.data + 98, part2.size);
            //
            ////printf("%d", resp.size);

            //chrs[2] = 9;
            //chrs[1] = part1.size + 1;
            //memcpy(chrs + 3, part1.data, part1.size);
            //
            //send(clientSocket, chrs, chrs[1] + 2, 0);

            //chrs[2] = 99;
            //chrs[1] = part2.size + 1;
            //memcpy(chrs + 3 + 98, part2.data, part2.size);
            //send(clientSocket, chrs, chrs[1] + 2, 0);
            //
            ////printf("%d", part2.size);
            //free(part1.data);
            //free(part2.data);
            fldata:
            chrs[2] = 6;
            chrs[1] = 1;
            memcpy(&chrs[2 + chrs[1]], &lapsAndFuelData.previousLap, 4);
            chrs[1] += 4;
            memcpy(&chrs[2 + chrs[1]], &lapsAndFuelData.currentBestLap, 4);
            chrs[1] += 4;
            memcpy(&chrs[2 + chrs[1]], &lapsAndFuelData.leaderBestLap, 4);
            chrs[1] += 4;
            memcpy(&chrs[2 + chrs[1]], &lapsAndFuelData.allBestLap, 4);
            chrs[1] += 4;
            memcpy(&chrs[2 + chrs[1]], &lapsAndFuelData.allBestFuel, 4);
            chrs[1] += 4;
            send(clientSocketW, chrs, chrs[1] + 2, 0);
            
            if (map_buffer != NULL) {
                chrs[2] = 12;
                chrs[1] = 1;
                //printf("%d", map_buffer->num_cars);
                //memcpy(&chrs[2 + chrs[1]], &map_buffer->num_cars, 4);
                //r3e_int32 num = 21;
                //r3e_int32 num = 21 > map_buffer->num_cars ? map_buffer->num_cars : 21;
                //memcpy(&chrs[2 + chrs[1]], &num, 4);
                memcpy(&chrs[2 + chrs[1]], &map_buffer->num_cars, 4);
                chrs[1] += 4;
                send(clientSocketW, chrs, chrs[1] + 2, 0);
            }
        }
        //Sleep(1000);
        isClientConnected = TRUE;
        isClientNew = TRUE;
        currentState = -1;
        //memset(playerRatingInfoSended.ids, 0, sizeof(playerRatingInfoSended.ids));
        //playerRatingInfoSended.size = 0;
        struct timeval timeout;
        timeout.tv_sec = 1; // Timeout in seconds
        timeout.tv_usec = 500000;

        if (setsockopt(clientSocketW, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) == SOCKET_ERROR) {
            printf("setsockopt failed: %d\n", WSAGetLastError());
        }
        while (1) {

            // Receive data from the client
            memset(buffer, 0, sizeof(buffer));
            iResult = recv(clientSocketW, buffer, sizeof(buffer), 0);
            if (iResult > 0) {
                //int len = strlen(ascii_str);
                BYTE arr[200];
                
                const int msglen = buffer[1] & 0b01111111;
                int offset = 2;
                BYTE decoded[200];
                BYTE masks[4] = { buffer[offset], buffer[offset + 1], buffer[offset + 2], buffer[offset + 3] };
                offset += 4;
                //int con = 0;
                for (int i = 0; i < msglen + 1; ++i) {
                    if (i == msglen) {
                        decoded[i] = '\0';
                        break;
                    }
                    //con++;
                    //printf("%d: %d\n", con, (byte)(buffer[offset + i] ^ masks[i % 4]));
                    decoded[i] = (byte)(buffer[offset + i] ^ masks[i % 4]);
                }
                //printf("\n\nend\n\n");
                //printf("\n\n%s\n\n", decoded);
                //printf("\n\n%d\n\n", decoded[0]);
                if (buffer[0] == -120)
                    continue;
                if (buffer[0] == 107) {
                    isClientConnected = FALSE;
                    closesocket(clientSocketW);
                    clientSocketW = NULL;
                    break;
                }
                if (decoded[0] == 9) {
                    //printf("\n\n%d\n\n", msglen);
                    //writeWindowsSettings(&decoded[2], msglen);
                    memcpy(dataOfWinSizes, decoded + 2, 96);
                    lenOfWinSizes = 96;
                }
                if (decoded[0] == 99) {
                    //printf("\n\n%d\n\n", msglen);
                    memcpy(dataOfWinSizes + 96, decoded + 2, msglen);
                    writeWindowsSettings(&dataOfWinSizes, 96 + msglen);

                    //printf("\n");
                    //printf("Values in decimal:\n");
                    //for (int i = 0; i < msglen; i++) {
                    //    printf("%d: %d\n", i, (signed char)decoded[i]);  // Print as integer
                    //}
                    //printf("\n");

                    //printf("\n");
                    //printf("Values in decimal:\n");
                    ///*printf("0: 9\n");
                    //printf("1: 0\n");*/
                    //for (int i = 0; i < 104; i++) {
                    //    printf("%d: %d\n", i, (signed char)dataOfWinSizes[i]);  // Print as integer
                    //}
                    //printf("\n");

                    //printf("%s", dataOfWinSizes);
                    //printf("\n");
                    //printf("Values in decimal:\n");
                    //printf("0: 9\n");
                    //printf("1: 0\n");
                    //for (int i = 0; i < 106; i++) {
                    //    printf("%d: %d\n", i+2, (signed char)dataOfWinSizes[i]);  // Print as integer
                    //}
                    //printf("\n");
                }
                if (decoded[0] == 49) {
                    char* str = &decoded[1];

                    // Allocate memory for the name
                    playerRatingInfoDb.player[playerRatingInfoDb.size].name = malloc(64);

                    // Parse the string
                    sscanf_s(str, "%lu,%[^,],%d,%d", &playerRatingInfoDb.player[playerRatingInfoDb.size].id, playerRatingInfoDb.player[playerRatingInfoDb.size].name, 64, &playerRatingInfoDb.player[playerRatingInfoDb.size].rating, &playerRatingInfoDb.player[playerRatingInfoDb.size].reputation);

                    //printf("%s %d ", playerRatingInfoDb.player[playerRatingInfoDb.size].name, playerRatingInfoDb.player[playerRatingInfoDb.size].id);
                    if (strcmp(playerRatingInfoDb.player[playerRatingInfoDb.size].name, "None") == 0) {
                        char* result = playerRatingInfoDb.player[playerRatingInfoDb.size].name;
                        char* input = NULL;
                        //int id;

                        for (size_t i = 0; i < R3E_NUM_DRIVERS_MAX; i++) {
                            if (map_buffer->all_drivers_data_1[i].driver_info.user_id == playerRatingInfoDb.player[playerRatingInfoDb.size].id) {
                                input = map_buffer->all_drivers_data_1[i].driver_info.name;
                                //id = map_buffer->all_drivers_data_1[i].driver_info.user_id;
                                break;
                            }
                        }

                        if (input == NULL)
                            continue;
                        //playerRatingInfoDb.player[playerRatingInfoDb.size].id = id;
                        int i = 0, j = 0;

                        // Step 1: Get the first letter of the first part and add ". " to result
                        result[j++] = input[0];  // Add the first character
                        result[j++] = '.';       // Add the period
                        result[j++] = ' ';       // Add a space

                        // Step 2: Find the first space in the input
                        while (input[i] != ' ' && input[i] != '\0') {
                            i++;
                        }

                        // Step 3: Move `i` to point to the start of the second part
                        if (input[i] == ' ') {
                            i++;  // Move past the space character
                        }

                        // Step 4: Add up to 15 characters of the second part to `result`
                        int count = 0;
                        while (input[i] != '\0' && count < 15) {
                            result[j++] = input[i++];
                            count++;
                        }

                        // Step 5: Null-terminate the result string
                        result[j] = '\0';

                        int length = snprintf(NULL, 0, "%lu,%s,%d,%d", playerRatingInfoDb.player[playerRatingInfoDb.size].id, playerRatingInfoDb.player[playerRatingInfoDb.size].name, playerRatingInfoDb.player[playerRatingInfoDb.size].rating, playerRatingInfoDb.player[playerRatingInfoDb.size].reputation);
                        char* strr = malloc(length + 4);

                        // Create the string
                        //sprintf_s(str, length + 1, "%lu,%s,%d,%d", temp[playerRatingInfoDb.size].id, temp[playerRatingInfoDb.size].name, temp[playerRatingInfoDb.size].rating, temp[playerRatingInfoDb.size].reputation);
                        //sprintf_s(str, length + 1, "%lu,%s,%d,%d", temp->id, temp->name, temp->rating, temp->reputation);
                        //sprintf_s(&str[3], length + 1, "%lu,%s,%d,%d", temp->id, temp->name, temp->rating, temp->reputation);
                        sprintf_s(&strr[3], length + 1, "%lu,%s,%d,%d", playerRatingInfoDb.player[playerRatingInfoDb.size].id, playerRatingInfoDb.player[playerRatingInfoDb.size].name, playerRatingInfoDb.player[playerRatingInfoDb.size].rating, playerRatingInfoDb.player[playerRatingInfoDb.size].reputation);

                        //printf("%s\n", str);

                        strr[0] = 130;
                        strr[1] = length + 2;
                        strr[2] = 5;
                        //printf("%s", strr);
                        sendMessage(strr, length + 4);
                        free(strr);
                    }
                    //printf("\n");
                    playerRatingInfoDb.size++;
                }
                if (decoded[0] == 3) {
                    isClientConnected = FALSE;
                    closesocket(clientSocketW);
                    clientSocketW = NULL;
                    break;
                }

            }
            else if (iResult == 0) {
                isClientConnected = FALSE;
                clientSocketW = NULL;
                //printf("Connection closed.\n");
                break;
            }
            else if (!clientSocketW) {
                break;
            }
            else if (iResult == -1) {
                continue;
            }
            else {
                isClientConnected = FALSE;
                printf("recv failedW: %d, %d\n", WSAGetLastError(), iResult);
                closesocket(clientSocketW);
                //WSACleanup();
                clientSocketW = NULL;
                break;
                //return 1;
            }

            // Close the client socket
            //closesocket(clientSocket);
        }
    }
    // Close the server socket and cleanup Winsock
    closesocket(serverSocket);
    WSACleanup();
    

    return 0;
}

void sendMessage(const unsigned char* chars[], int size) {
    if (clientSocketW) {
        //printf("sended %s\n", chars);
        send(clientSocketW, chars, size/*strlen(chars)*/, 0);
        //int iResult = send(clientSocket, chars, size/*strlen(chars)*/, 0);
        //printf("sended %d\n", iResult);
    }
}

void startServers() {
    checkIsHtmlsAreExistsAndCreateThemIfNotOrFillTheStringsWithIt();
    HANDLE thread_handle = NULL;
    DWORD thread_id;

    thread_handle = CreateThread(NULL, 0, thread_function, NULL, 0, &thread_id);

    if (thread_handle == NULL) {
        printf("Thread creation failed\n");
        //return 1;
    }

    // Wait for the thread to finish
    //WaitForSingleObject(thread_handle, INFINITE);

    // Close the thread handle
    //CloseHandle(thread_handle);
    
    HANDLE thread_handle1 = NULL;
    DWORD thread_id1;

    thread_handle1 = CreateThread(NULL, 0, thread_function1, NULL, 0, &thread_id1);

    if (thread_handle1 == NULL) {
        printf("Thread creation failed\n");
        //return 1;
    }

    // Wait for the thread to finish
    //WaitForSingleObject(thread_handle1, INFINITE);

    // Close the thread handle
    //CloseHandle(thread_handle1);
}

void checkIsHtmlsAreExistsAndCreateThemIfNotOrFillTheStringsWithIt() {
    //FILE* file;
    //char* filename = "index.html";
    //char line[256];  // Buffer to hold each line

    //errno_t err = fopen_s(&file, filename, "r");
    //if (err == 0) {
    //    while (fgets(line, sizeof(line), file)) {
    //        printf("%s", line);
    //    }
    //    fclose(file);
    //}
    //else {
    //    printf("Error opening file!\n");
    //}
    long length;

    FILE* file;
    char* filename = "index.html";

    errno_t err = fopen_s(&file, filename, "rb");
    if (err == 0) {
        fseek(file, 0, SEEK_END);
        length = ftell(file);
        fseek(file, 0, SEEK_SET);
        if (html) {
            free(html);
            html = NULL;
        }
        html = malloc(length + 1);
        if (html) {
            fread(html, 1, length, file);
            html[length] = '\0';
            buildResponz();
        }
        fclose(file);
    }
    else {
        printf("File does not exist. Creating and writing data to the file.\n");
        buildHtml();
        buildResponz();
        err = fopen_s(&file, filename, "wb");
        if (err == 0) {
            fwrite(html, sizeof(char), strlen(html), file);
            fclose(file);
        }
        else {
            printf("Error opening file!\n");
        }
    }

}

void buildHtml() {

    //int totalLength = strlen(responz1) + strlen(responz2) + strlen(responz3) + strlen(responz4) + strlen(responz5) + strlen(responz6) + strlen(responz7) + strlen(responz8) + strlen(responz9) + 1;
    int totalLength = strlen(responz1) + strlen(responz2) + strlen(responz3) + strlen(responz4) + strlen(responz5) + strlen(responz6) + strlen(responz7) + 1;
    if (html) {
        free(html);
        html = NULL;
    }
    html = malloc(totalLength * sizeof(char));
    //printf("%s", responz);
    //printf("%d", strlen(responz3));
    if (html == NULL) {
        printf("responz malloc failed");
    }
    else {
        // Initialize the new string
        html[0] = '\0';

        // Concatenate the strings
        strcat_s(html, totalLength, responz1);
        strcat_s(html, totalLength, responz2);
        strcat_s(html, totalLength, responz3);
        strcat_s(html, totalLength, responz4);
        strcat_s(html, totalLength, responz5);
        strcat_s(html, totalLength, responz6);
        strcat_s(html, totalLength, responz7);
        /*strcat_s(html, totalLength, responz8);
        strcat_s(html, totalLength, responz9);*/
        //printf("%d", totalLength);
        html[totalLength] = '\0';
    }
    //printf("%s", html);
}

char* responzz;

void buildResponz() {
    int totalLength = strlen(response) + strlen(html) + 1;
    if (responz) {
        free(responz);
        responz = NULL;
    }
    if (responzz) {
        free(responzz);
        responzz = NULL;
    }
    responz = malloc(totalLength * sizeof(char));
    responzz = malloc(totalLength * sizeof(char) + 10);

    if (responz == NULL) {
        printf("responz malloc failed");
    }
    else {
        responz[0] = '\0';

        strcat_s(responz, totalLength, response);
        strcat_s(responz, totalLength, html);
        
        char* pos1 = strstr(responz, "8082");

        if (pos1) {
            size_t lenBefore = pos1 - responz;
            size_t lenAfter = strlen(pos1 + strlen("8082"));

            snprintf(responzz, totalLength * sizeof(char) + 10, "%.*s%s%s",
                (int)lenBefore, responz, settings.wesocket_port, pos1 + strlen("8082"));
            free(responz);
            responz = responzz;
            responzz = NULL;
        }
    }
}