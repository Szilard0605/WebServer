
let PlayerID = 0;
let IsHost = false;

let CanvasWidth = 800;
let CanvasHeight = 600; 

let BarWidth = CanvasWidth / 15;
let BarHeight = CanvasHeight / 3;

let Player0_Y = CanvasHeight / 2 - BarHeight / 2;
let Player1_Y = CanvasHeight / 2 - BarHeight / 2;

let BallSize = 30;

let BallPosX = CanvasWidth  / 2 - BallSize / 2;
let BallPosY = CanvasHeight / 2 - BallSize / 2;

const moveSpeed = 16.0;

let connected = false;

let touchEnded = true;
let touchY = 0;

let fetchRate = 60;

window.requestAnimationFrame(gameLoop);

// Gets called on body onload
function InitFamPong()
{
    // Make an AJAX request to fetch JSON data from the server
    const xhr = new XMLHttpRequest();

        xhr.onreadystatechange = function() 
        {
            console.log("blud is" + xhr.readyState);

            if (xhr.readyState === XMLHttpRequest.DONE) 
            {
                if (xhr.status === 200) 
                {
                    const responseJson = JSON.parse(xhr.responseText);
                    // Update PlayerID and IsHost variables
                    PlayerID = responseJson.PlayerID;
                    IsHost = responseJson.IsHost;
                    // You can now use PlayerID and IsHost variables as needed
                    console.log('PlayerID:', PlayerID);
                    console.log('IsHost:', IsHost);
                
                    //let dbgtext = document.getElementById("debugtext");
                    //dbgtext.innerHTML = IsHost ? "Host vagy" : "Player" + PlayerID + " vagy";

                    connected = true;
                
                    console.log("connected to FamPong webserver");
                } 
                else 
                {
                    console.error('Failed to fetch data from server');
                }
            }
        };
        xhr.open('GET', 'http://192.168.0.34:7000/fampong_connect', true); // Replace with your server address
        xhr.send();
    
    var c = document.getElementById("maincanvas");
    var ctx = c.getContext("2d");
    ctx.canvas.width = CanvasWidth;
    ctx.canvas.height = CanvasHeight;
    c.addEventListener("mousedown", CanvasMouseDown, false);
    c.addEventListener("touchstart", CanvasTouchStart, false);
    c.addEventListener("touchend", CanvasTouchEnd, false);
}

function QuitFamPong()
{
    
}

function CanvasTouchEnd(event)
{
    if(IsHost)
        return;

    touchEnded = true;
    event.preventDefault();
}

function CanvasTouchStart(event)
{
    //if(!connected)
    //    return;

    if(IsHost)
        return;

    touchEnded = false;


    var c = document.getElementById("maincanvas");
    let bound = c.getBoundingClientRect();

    let x = event.touches[0].clientX - bound.left - c.clientLeft;
    touchY = event.touches[0].clientY - bound.top - c.clientTop;


    event.preventDefault();
}

function CanvasMouseDown(event)
{
    if(!connected)
        return;

    if(event.button != 0)
        return;

    var c = document.getElementById("maincanvas");
    let bound = c.getBoundingClientRect();

    let x = event.clientX - bound.left - c.clientLeft;
    let y = event.clientY - bound.top - c.clientTop;

    if(IsHost == false)
    {
        if(y < CanvasHeight / 2)
        {
            console.log("UP");
            sendMoveDataToServer(1);
        }
        else
        {
            console.log("DOWN");
            sendMoveDataToServer(0);
        }
    }
}

// move: 1 - up, 0 - down
function sendMoveDataToServer(move)
{
    const data = {
        PlayerID: PlayerID,
        Movement: move
    };


    const xhr = new XMLHttpRequest();

    console.log("Sent:" + JSON.stringify(data));

    xhr.open('POST', 'http://192.168.0.34:7000/fampong_move', true); // Replace with your server address
    xhr.setRequestHeader('Content-Type', 'application/json');
    xhr.send(JSON.stringify(data));
}

function FetchMovement()
{
    if(!connected)
        return;

    if(IsHost == true)
    {
       
           // Make an AJAX request to fetch JSON data from the server
        const xhr = new XMLHttpRequest();
        xhr.onreadystatechange = function() 
        {
            if (xhr.readyState === XMLHttpRequest.DONE) 
            {
                if (xhr.status === 200) 
                {
      
                    const responseJson = JSON.parse(xhr.responseText);

             
                    // Update PlayerID and IsHost variables
                    let playerid = responseJson.PlayerID;
                    let move = responseJson.Movement;
                    
                    if(move == -1)
                    {
                        return;
                    }
                    //console.log("playerid:  " + playerid + ", movement: " + move);

                    if(playerid === 1)
                    {
                        if(move == 0)
                            Player0_Y += moveSpeed;
                        else
                            Player0_Y -= moveSpeed
                    }

                    if(playerid === 2)
                    {
                    
                        if(move == 0)
                            Player1_Y += moveSpeed;
                        else
                            Player1_Y -= moveSpeed
                    }
                } 
                else 
                {
                    console.error('Failed to fetch data from server');
                }
            }
        };

        xhr.open('GET', 'http://192.168.0.34:7000/fampong_move', true); // Replace with your server address
        xhr.setRequestHeader('Content-Type', 'application/json');
        xhr.send();
    }
}

// Periodically fetch movement data from the server
setInterval(FetchMovement, fetchRate); // Adjust interval as needed

function gameLoop()
{
    if(IsHost)
    {
        renderGame()
    }
    else
    {    
        if(!touchEnded)
        {
            if(touchY < CanvasHeight / 2)
            {
                sendMoveDataToServer(1);
            }
            else
            {
                sendMoveDataToServer(0);
            }
        }

        renderControls();
    }

    window.requestAnimationFrame(gameLoop);
}

function renderGame()
{
    var c = document.getElementById("maincanvas");
    var ctx = c.getContext("2d");

    ctx.clearRect(0, 0, CanvasWidth, CanvasHeight);

    ctx.beginPath();

    ctx.fillStyle = "#00FF00";
    ctx.fillRect(0, Player0_Y, BarWidth, BarHeight);

    ctx.fillStyle = "#0000FF";
    ctx.fillRect(CanvasWidth - BarWidth, Player1_Y, BarWidth, BarHeight);

    ctx.fillStyle = "#FFFFFF";
    ctx.fillRect(BallPosX, BallPosY, 30, 30);
}

function renderControls()
{
    var c = document.getElementById("maincanvas");
    var ctx = c.getContext("2d");
    ctx.clearRect(0, 0, CanvasWidth, CanvasHeight);

    ctx.beginPath();

    ctx.fillStyle = "#00FF00";
    ctx.fillRect(0, 0, CanvasWidth, CanvasHeight / 2);

    ctx.fillStyle = "#FF0000";
    ctx.fillRect(0, CanvasHeight / 2, CanvasWidth, CanvasHeight);

    ctx.fillStyle = "#FFFFFF";
    ctx.font = "150px serif";
    ctx.textAlign = "center";
    ctx.fillText("↑", CanvasWidth / 2, CanvasHeight / 3);

    ctx.fillStyle = "#FFFFFF";
    ctx.font = "150px serif";
    ctx.textAlign = "center";
    ctx.fillText("↓", CanvasWidth / 2, CanvasHeight / 2 + CanvasHeight / 3);


    let text = touchEnded;

    ctx.fillStyle = "#FFFFFF";
    ctx.font = "150px serif";
    ctx.textAlign = "center";
    ctx.fillText(touchEnded, CanvasWidth / 2, CanvasHeight / 2);

    ctx.fillStyle = "#FFFFFF";
    ctx.font = "150px serif";
    ctx.textAlign = "center";
    ctx.fillText(touchY, CanvasWidth / 2, CanvasHeight / 2 + 130);

}

// Gets called on body onunload
function QuitFamPong()
{

}