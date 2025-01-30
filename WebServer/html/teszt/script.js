let gridSize = 40;
const snakeSize = gridSize;
let snake = {
    pos: [0, 0],
    dir: [0, 0],
};

let tailCount = 0;

let tails = [{
    pos: [0, 0],
    dir: [0, 0]
}];

let apple = {
    pos: []
};

let score = 0;
let scoreText;

function drawGrid() {
    for (i = 0; i < width; i++) {
        if (i % gridSize == 0) {
            line(i, 0, i, height);
            line(0, i, width, i);
        }

    }
}

let lstmoveTime = 0;

function randomApplePos() {
    apple.pos.x = floor(random(0, width / gridSize)) * gridSize;
    apple.pos.y = floor(random(0, width / gridSize)) * gridSize;
}

function moveSnake() {
    // moving the snake
    if (keyIsDown(LEFT_ARROW) && snake.dir.x !== 1) {
        snake.dir.x = -1;
        snake.dir.y = 0;
    }
    else if (keyIsDown(RIGHT_ARROW) && snake.dir.x !== -1) {
        snake.dir.x = 1;
        snake.dir.y = 0;
    }
    else if (keyIsDown(UP_ARROW) && snake.dir.y !== 1) {
        snake.dir.x = 0;
        snake.dir.y = -1;
    }
    else if (keyIsDown(DOWN_ARROW) && snake.dir.y !== -1) {
        snake.dir.x = 0;
        snake.dir.y = 1;
    }

    if (snake.dir.x == 0 && snake.dir.y == 0)
        return;

    if (millis() - lstmoveTime >= 100) {
        lstmoveTime = millis();
        let spX = snake.pos.x;
        let spY = snake.pos.y;
        let sdX = snake.dir.x;
        let sdY = snake.dir.y;

        snake.pos.x += snake.dir.x * gridSize;
        snake.pos.y += snake.dir.y * gridSize;

        if (tailCount === 0)
            return;

        tails[0].pos.x = spX;
        tails[0].pos.y = spY;
        tails[0].dir.x = sdX;
        tails[0].dir.y = sdY;


        for (i = 1; i <= tailCount; i++) {

            let ppX = tails[i].pos.x;
            let ppY = tails[i].pos.y;
            let pdX = tails[i].dir.x;
            let pdY = tails[i].dir.y;

            tails[i].pos.x = spX;
            tails[i].pos.y = spY;
            tails[i].dir.x = sdX;
            tails[i].dir.y = sdY;

            spX = ppX;
            spY = ppY;
            sdX = pdX;
            sdY = pdY;

            if (snake.pos.x == tails[i].pos.x && snake.pos.y == tails[i].pos.y) {
                tailCount = 0;
                resetGame();
                return;
            }
        }
    }

    if (snake.pos.x + gridSize / 2 > width) {
        snake.pos.x = 0;
    }
    if (snake.pos.x + gridSize / 2 < 0) {
        console.log("should go otherside");
        snake.pos.x = height - gridSize;
    }
    if (snake.pos.y + gridSize / 2 > height) {
        snake.pos.y = 0;
    }
    if (snake.pos.y + gridSize / 2 < 0) {
        snake.pos.y = height - gridSize;
    }

    if (snake.pos.x === apple.pos.x && snake.pos.y === apple.pos.y) {
        randomApplePos();

        let tpX = tails[tailCount].pos.x - tails[tailCount].dir.x * gridSize;
        let tpY = tails[tailCount].pos.y - tails[tailCount].dir.y * gridSize;
        let tdX = tails[tailCount].dir.x;
        let tdY = tails[tailCount].dir.y;

        tailCount++;

        tails.push({ pos: { x: tpX, y: tpY }, dir: { x: tdX, y: tdY } });

        score++;
    }
}

function drawSnake() {
    // drawing the snake
    fill(color(0, 200, 0, 255));
    rect(snake.pos.x, snake.pos.y, gridSize);
}

function drawTails() {
    for (i = 0; i <= tailCount; i++) {
        // drawing the snake
        fill(color(0, 150, 0, 255));
        rect(tails[i].pos.x, tails[i].pos.y, gridSize);
    }
}

function drawApple() {
    // drawing the snake
    fill(color(255, 0, 0, 255));
    rect(apple.pos.x, apple.pos.y, gridSize);
}

function resetGame() {
    score = 0;

    snake.dir.x = 0;
    snake.dir.y = 0;

    tails = [{
        pos: [],
        dir: []
    }];
    // set snake position into the center cell
    let gridRowCount = width / gridSize;
    let gridColCount = height / gridSize;
    snake.pos.x = floor(gridSize * gridRowCount / 2);
    snake.pos.y = floor(gridSize * gridColCount / 2);

    randomApplePos();

}

function centerCanvas(canvas) {
    // Center the canvas in the window
    let x = (windowWidth - width) / 2;
    let y = (windowHeight - height) / 2;
    canvas.position(x, y);
}

function setup() {
    let canvas = createCanvas(720, 720);
    centerCanvas(canvas);
    scoreText = createP('Score: ');
    // Style the paragraph element to position it on top of the canvas
    scoreText.style('position', 'fixed');
    scoreText.style('left', '50%');
    scoreText.style('top', '10%'); // Adjust top to place it above the canvas
    scoreText.style('transform', 'translateX(-50%)'); // Center horizontally
    scoreText.style('color', '#ff0000'); // Optional: set the text color
    scoreText.style('font-size', '48px'); // Optional: set the font size
    scoreText.style('pointer-events', 'none'); // Allow mouse events to pass through the paragraphh
    resetGame();
}

function windowResized() {
    centerCanvas();
}

function draw() {
    background(220);
    moveSnake();
    drawGrid();
    drawSnake();
    drawTails();
    drawApple();

    scoreText.html("Score: " + score);
}