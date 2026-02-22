const bytes = await (await fetch("./cgol.wasm")).arrayBuffer();
const { instance } = await WebAssembly.instantiate(bytes, {});
const e = instance.exports;

const canvas = document.getElementById("canvas");
const ctx = canvas.getContext("2d");

const offscreen = document.createElement("canvas");
const offscreenCtx = offscreen.getContext("2d");
let cgolImage = offscreenCtx.createImageData(offscreen.width, offscreen.height);

function onResize() {
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
    
    // Disable image smoothing for pixelated/nearest-neighbor scaling
    // Must be set after resizing as it resets the context
    ctx.imageSmoothingEnabled = false;
    e.init_state(canvas.width, canvas.height, Math.random() * 100_000);
    
    // Resize offscreen canvas to match CGOL state size
    offscreen.width = e.get_state_width();
    offscreen.height = e.get_state_height();
    cgolImage = offscreenCtx.createImageData(offscreen.width, offscreen.height);
}
window.addEventListener("resize", onResize);
onResize();

const draw = () => {
    e.update_state();

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
