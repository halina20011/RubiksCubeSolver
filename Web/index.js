const vertexShaderSource = `#version 300 es
in vec3 position;
in vec3 color;

uniform mat4 modelMatrix;
uniform mat4 viewMat;
uniform mat4 projectionMat;

uniform vec3 cUniform;

out vec3 Color;
void main(){
    gl_Position = projectionMat * viewMat * modelMatrix * vec4(position, 1);
    Color = color * cUniform;
}`;

const fragmentShaderSource = `#version 300 es
precision highp float;
in vec3 Color;
out vec4 color;

void main(){
    color = vec4(Color, 1);
}`;

function createShader(gl, shaderSource, type){
    const shader = gl.createShader(type);
    gl.shaderSource(shader, shaderSource);
    gl.compileShader(shader);
    if(!gl.getShaderParameter(shader, gl.COMPILE_STATUS)){
        console.error(gl.getShaderInfoLog(shader));
        gl.deleteShader(shader);
        return null;
    }
    
    return shader;
}

function createProgram(gl, vertexShader, fragmentShader){
    const program = gl.createProgram();
    gl.attachShader(program, vertexShader);
    gl.attachShader(program, fragmentShader);
    gl.linkProgram(program);
    if(!gl.getProgramParameter(program, gl.LINK_STATUS)){
        console.error("failed to link program");
        return null;
    }

    return program;
}

function getUniformLocation(gl, program, uniformName, name, data){
    if(name == null){
        name = uniformName;
    }

    data[name] = gl.getUniformLocation(program, uniformName);
}

const cube = new Float32Array([
    -0.5, -0.5, -0.5, 1.0, 1.0, 0.0,
     0.5, -0.5, -0.5, 1.0, 1.0, 0.0,
     0.5,  0.5, -0.5, 1.0, 1.0, 0.0,
     0.5,  0.5, -0.5, 1.0, 1.0, 0.0,
    -0.5,  0.5, -0.5, 1.0, 1.0, 0.0,
    -0.5, -0.5, -0.5, 1.0, 1.0, 0.0,

    -0.5, -0.5,  0.5, 1.0, 1.0, 0.0,
     0.5, -0.5,  0.5, 1.0, 1.0, 0.0,
     0.5,  0.5,  0.5, 1.0, 1.0, 0.0,
     0.5,  0.5,  0.5, 1.0, 1.0, 0.0,
    -0.5,  0.5,  0.5, 1.0, 1.0, 0.0,
    -0.5, -0.5,  0.5, 1.0, 1.0, 0.0,

    -0.5,  0.5,  0.5, 1.0, 1.0, 0.0,
    -0.5,  0.5, -0.5, 1.0, 1.0, 0.0,
    -0.5, -0.5, -0.5, 1.0, 1.0, 0.0,
    -0.5, -0.5, -0.5, 1.0, 1.0, 0.0,
    -0.5, -0.5,  0.5, 1.0, 1.0, 0.0,
    -0.5,  0.5,  0.5, 1.0, 1.0, 0.0,

     0.5,  0.5,  0.5, 1.0, 1.0, 0.0,
     0.5,  0.5, -0.5, 1.0, 1.0, 0.0,
     0.5, -0.5, -0.5, 1.0, 1.0, 0.0,
     0.5, -0.5, -0.5, 1.0, 1.0, 0.0,
     0.5, -0.5,  0.5, 1.0, 1.0, 0.0,
     0.5,  0.5,  0.5, 1.0, 1.0, 0.0,

    -0.5, -0.5, -0.5, 1.0, 1.0, 0.0,
     0.5, -0.5, -0.5, 1.0, 1.0, 0.0,
     0.5, -0.5,  0.5, 1.0, 1.0, 0.0,
     0.5, -0.5,  0.5, 1.0, 1.0, 0.0,
    -0.5, -0.5,  0.5, 1.0, 1.0, 0.0,
    -0.5, -0.5, -0.5, 1.0, 1.0, 0.0,

    -0.5,  0.5, -0.5, 1.0, 1.0, 0.0,
     0.5,  0.5, -0.5, 1.0, 1.0, 0.0,
     0.5,  0.5,  0.5, 1.0, 1.0, 0.0,
     0.5,  0.5,  0.5, 1.0, 1.0, 0.0,
    -0.5,  0.5,  0.5, 1.0, 1.0, 0.0,
    -0.5,  0.5, -0.5, 1.0, 1.0, 0.0
]);

let modelParsed = false;
const sizes = [];
const datas = [];
// const MODEL = "cmodel.bin";
const MODEL = "model.bin";
function loadModel(){
    fetch(MODEL).then(r => r.blob().then(b => {
        console.log(b.size);
        console.log(b);

        const sMap = new Map();
        const reader = new FileReader();
        reader.onload = function(){
            const arrBuffer = reader.result;
            const dataView = new DataView(arrBuffer);
            let offset = 0;
            while(offset < arrBuffer.byteLength){
                const size = dataView.getUint32(offset, true);
                console.log(`size: ${size}`);
                sizes.push(size);
                if(!sMap.has(size)){
                    sMap.set(size, {count: 0});
                }
                sMap.get(size).count++;
                offset += 4;

                const data = new Array(size);
                for(let i = 0; i < size; i++){
                    data[i] = dataView.getFloat32(offset, true);
                    // console.log(data[i]);
                    offset += 4;
                }

                datas.push(data);
                // console.log(data);
                // return data;
            }
            const data = new Float32Array(datas.flat())
            gl.bufferData(gl.ARRAY_BUFFER, data, gl.STATIC_DRAW);
            modelParsed = true;
            console.log(sMap);
        }

        reader.readAsArrayBuffer(b);
    }));
}

function webglInit(){
    const canvas = document.querySelector("canvas");
    const gl = canvas.getContext("webgl2");
    if(!gl){
        const errorMessage = "failed to get webgl2 context";
        console.error(errorMessage);
        alert(errorMessage);
    }

    gl.enable(gl.DEPTH_TEST);

    const vertexShader = createShader(gl, vertexShaderSource, gl.VERTEX_SHADER);
    const fragmentShader = createShader(gl, fragmentShaderSource, gl.FRAGMENT_SHADER);

    const program = createProgram(gl, vertexShader, fragmentShader);

    const positionAttribute = gl.getAttribLocation(program, "position");
    const colorAttribute = gl.getAttribLocation(program, "color");

    const uniforms = {};
    getUniformLocation(gl, program, "modelMatrix", null, uniforms);
    getUniformLocation(gl, program, "projectionMat", null, uniforms);
    getUniformLocation(gl, program, "viewMat", null, uniforms);
    getUniformLocation(gl, program, "cUniform", null, uniforms);

    const buffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, buffer);

    const vao = gl.createVertexArray();
    gl.bindVertexArray(vao);
    gl.enableVertexAttribArray(positionAttribute);
    gl.enableVertexAttribArray(colorAttribute);

    const type = gl.FLOAT;
    const normalize = false;
    const stride = 6 * Float32Array.BYTES_PER_ELEMENT;
    const colorOffset = 3 * Float32Array.BYTES_PER_ELEMENT;
    gl.vertexAttribPointer(positionAttribute, 3, type, normalize, stride, 0);
    gl.vertexAttribPointer(colorAttribute, 3, type, normalize, stride, colorOffset);

    gl.useProgram(program);
    gl.bindVertexArray(vao);

    gl.clearColor(0, 0, 0, 0);
    
    return [gl, program, uniforms];
}

const projectionMat = glMatrix.mat4.create();
// glMatrix.mat4.identity(projectionMat);
// console.log(projectionMat);
// const fov = (Math.PI / 180) * 45;
const fov = Math.PI / 4;
glMatrix.mat4.perspective(projectionMat, fov, 1.0, 0.01, 100);
// console.log(projectionMat);

function draw(gl, uniforms){
    const cameraPos = glMatrix.vec3.fromValues(0, 0, 3);
    
    const cameraFront = glMatrix.vec3.create(0, 0, -1);
    const cameraUp = glMatrix.vec3.create(0, 1, 0);
    const cameraCenter = glMatrix.vec3.create();

    glMatrix.vec3.add(cameraCenter, cameraPos, cameraFront);

    const viewMat = glMatrix.mat4.create();
    glMatrix.mat4.identity(viewMat);
    
    glMatrix.mat4.lookAt(viewMat, cameraCenter, cameraCenter, cameraUp);
    // console.log(viewMat);

    const modelMatrix = glMatrix.mat4.create();
    glMatrix.mat4.identity(modelMatrix);

    const xV  = parseFloat(x.value);
    const yV  = parseFloat(y.value);
    const zV  = parseFloat(z.value);
    
    let xSV  = parseFloat(xS.value);
    // const ySV  = parseFloat(yS.value);
    // const zSV  = parseFloat(zS.value);
    
    const xRV  = parseFloat(xR.value);
    const yRV  = parseFloat(yR.value);
    const zRV  = parseFloat(zR.value);
    // console.log(xV, yV, zV, xRV, yRV, zRV);
    const posVec = glMatrix.vec3.fromValues(xV, yV, zV);
    // const rotVec = glMatrix.vec3.fromValues(xRV, yRV, zRV);
    // glMatrix.mat4.rotate(modelMatrix, modelMatrix, rotVec);
    glMatrix.mat4.translate(modelMatrix, modelMatrix, posVec);
    glMatrix.mat4.rotateX(modelMatrix, modelMatrix, xRV);
    glMatrix.mat4.rotateY(modelMatrix, modelMatrix, yRV);
    glMatrix.mat4.rotateZ(modelMatrix, modelMatrix, zRV);

    if(xSV == 0){
        xSV = 1;
    }
    const scale = glMatrix.vec3.fromValues(xSV, xSV, xSV);
    glMatrix.mat4.scale(modelMatrix, modelMatrix, scale);

    gl.uniformMatrix4fv(uniforms["modelMatrix"], false, modelMatrix);

    gl.uniformMatrix4fv(uniforms["viewMat"], false, viewMat);
    gl.uniformMatrix4fv(uniforms["projectionMat"], false, projectionMat);

    gl.clear(gl.COLOR_BUFFER_BIT);
    const COLOR = new Float32Array([1, 1, 1]);
    const WIREFRAME = new Float32Array([0, 0, 0]);

    gl.uniform3fv(uniforms["cUniform"], COLOR);
    if(modelParsed){
        let offset = 0;
        const from = 1;
        for(let i = 0; i < sizes.length; i++){
            const size = sizes[i] / 6;
            if(from <= i){
                // console.log('dfsdfsdfs');
                // console.log(datas[i]);
                gl.drawArrays(gl.TRIANGLES, offset, size);
            }
            offset += size;
            // break;
        }

        offset = 0;
        gl.uniform3fv(uniforms["cUniform"], WIREFRAME);
        for(let i = 0; i < sizes.length; i++){
            const size =  sizes[i] / 6;
            for(let t = 0; t < size / 3; t++){
                gl.drawArrays(gl.LINE_STRIP, 3 * t, 3);
            }
        }
    }

}

setInterval(() => {
    draw(gl, uniforms);
}, 100);

const [gl, program, uniforms] = webglInit();
loadModel();
draw(gl, uniforms);
