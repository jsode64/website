// Ratio of pixels per cell.
const CELL_RATIO = 4;

// Set up reset button.
const resetButton = document.getElementsByClassName("reset-button")[0];
resetButton.onclick = () => {
    e.randomize_state();
};

// Load WASM.
const bytes = await (await fetch("./cgol.wasm")).arrayBuffer();
const { instance } = await WebAssembly.instantiate(bytes, {});
const e = instance.exports;

const canvas = document.getElementById("canvas");
const ctx = canvas.getContext("2d");

const offscreen = document.createElement("canvas");
const offscreenCtx = offscreen.getContext("2d");
let cgolImage = offscreenCtx.createImageData(offscreen.width, offscreen.height);

// Resize the canvas and CGoL state.
function onResize() {
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;

    ctx.imageSmoothingEnabled = false;
    e.resize_state(canvas.width / CELL_RATIO, canvas.height / CELL_RATIO, Date.now());
    
    offscreen.width = e.get_state_width();
    offscreen.height = e.get_state_height();
    cgolImage = offscreenCtx.createImageData(offscreen.width, offscreen.height);
}
window.addEventListener("resize", onResize);
onResize();

const draw = () => {
    e.update_state();

    // Get image from CGoL state.
    const pixels = new Uint8ClampedArray(
        e.memory.buffer,
        e.get_state_data(),
        offscreen.width * offscreen.height * 4,
    )
    cgolImage.data.set(pixels);
    offscreenCtx.putImageData(cgolImage, 0, 0);
    
    // Scale to fill entire canvas
    ctx.drawImage(offscreen, 0, 0, canvas.width, canvas.height);

    requestAnimationFrame(draw);
};

draw();
