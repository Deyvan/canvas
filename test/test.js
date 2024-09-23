// that file will be removed

let canvas = require("../skia.linux-x64-gnu.node")

for(let gpu of canvas.getGPUs()){
    console.log(gpu, gpu.name)
}