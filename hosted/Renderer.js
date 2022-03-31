
var renderer ;
class Renderer{

    gl; // WebGl instance
    shaderProgram; 

    camera_pos;
    mvMatrix = mat4.create(); // Model view matrix
    pMatrix = mat4.create();  // Perspective matrix
    bgColor = [220,220,220];

    buffers = {} ; // current active mesh buffers maps id to object with position,color, and normal
 
    start_camera ; // A clone of the camera mvMatrix made when a camera operation starts
    action_focus = [0,0,0]; // the focal point of the current camera operation
    start_pointer ; // Mouse or touch pointers saved whena camera operation starts

    maxPanSpeed = 5;
    xOrbitSpeed=-0.004;
    yOrbitSpeed=0.007;
    rotate_speed=0.01;

    
    // Performs the set-up for openGL canvas and shaders on construction
    constructor(webgl_canvas_id, ui_canvas_id , fragment_shader_id, vertex_shader_id, space_underneath_app){
        renderer = this ;
        var canvas = document.getElementById(webgl_canvas_id);
        var ui_canvas = document.getElementById(ui_canvas_id);
        canvas.width = document.body.clientWidth; 
        canvas.height = document.body.clientHeight-space_underneath_app; 
        ui_canvas.width = document.body.clientWidth; 
        ui_canvas.height = document.body.clientHeight-space_underneath_app; 
        
        this.initGL(canvas);
        this.initShaderProgram(fragment_shader_id, vertex_shader_id);
        this.gl.clearColor(0.0, 0.0, 0.0, 1.0);
        this.gl.enable(this.gl.DEPTH_TEST);
        this.gl.enable(this.gl.CULL_FACE);
        this.gl.cullFace(this.gl.BACK);
        console.log(this.gl.getParameter(this.gl.VERSION));
        console.log(this.gl.getParameter(this.gl.SHADING_LANGUAGE_VERSION));
        console.log(this.gl.getParameter(this.gl.VENDOR));
        this.gl.getExtension('OES_texture_float');
        this.setLightPosition([0,0,-250]);

        mat4.perspective(this.pMatrix, 45, this.gl.viewportWidth / this.gl.viewportHeight, 0.1, 3000.0);
        this.camera_pos = [1,1,1];
        mat4.lookAt(this.mvMatrix, this.camera_pos, [0,0,0], [0,1,0] );

        //console.log(this.gl);
        
    }

    // Initialize webGL on a canvas
    initGL(canvas){
        try {
            this.gl = canvas.getContext("webgl2",{ xrCompatible: true });
            this.gl.viewportWidth = canvas.width;
            this.gl.viewportHeight = canvas.height;
        } catch (e) {
        }
        if (!this.gl) {
            alert("Could not initialise WebGL, sorry :-(");
        }
    }

    // Fetches and compiles a shader script from a page element.
    // Used by initShaderProgram which is probably what you want to call.
    getShader(gl, id) {
        var shaderScript = document.getElementById(id);
        if (!shaderScript) {
            return null;
        }
        var str = "";
        var k = shaderScript.firstChild;
        while (k) {
            if (k.nodeType == 3) {
                str += k.textContent;
            }
            k = k.nextSibling;
        }
        var shader;
        if (shaderScript.type == "x-shader/x-fragment") {
            shader = gl.createShader(gl.FRAGMENT_SHADER);
        } else if (shaderScript.type == "x-shader/x-vertex") {
            shader = gl.createShader(gl.VERTEX_SHADER);
        } else {
            return null;
        }
        gl.shaderSource(shader, str);
        gl.compileShader(shader);
        if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
            console.log(gl.getShaderInfoLog(shader));
            return null;
        }
        return shader;
    }

    // Initialize shaders defined in html script elements into shaderProgram for webGL
    //TODO this is matched to a specific simple sahder, should be generalized
    initShaderProgram(fragment_shader_id, vertex_shader_id) {
        var fragmentShader = this.getShader(this.gl, fragment_shader_id);
        var vertexShader = this.getShader(this.gl, vertex_shader_id);

        this.shaderProgram = this.gl.createProgram();
        this.gl.attachShader(this.shaderProgram, vertexShader);
        this.gl.attachShader(this.shaderProgram, fragmentShader);
        this.gl.linkProgram(this.shaderProgram);

        if (!this.gl.getProgramParameter(this.shaderProgram, this.gl.LINK_STATUS)) {
            console.error("Could not initialize shaders!");
        }

        this.gl.useProgram(this.shaderProgram);
        this.shaderProgram.vertexPositionAttribute = this.gl.getAttribLocation(this.shaderProgram, "aVertexPosition");
        this.gl.enableVertexAttribArray(this.shaderProgram.vertexPositionAttribute);
        this.shaderProgram.vertexNormalAttribute = this.gl.getAttribLocation(this.shaderProgram, "aNormal");
        this.gl.enableVertexAttribArray(this.shaderProgram.vertexNormalAttribute);
        this.shaderProgram.vertexTexcoordAttribute = this.gl.getAttribLocation(this.shaderProgram, "aTexcoord");
        this.gl.enableVertexAttribArray(this.shaderProgram.vertexTexcoordAttribute);
        this.shaderProgram.vertexColorAttribute = this.gl.getAttribLocation(this.shaderProgram, "aVertexColor");
        this.gl.enableVertexAttribArray(this.shaderProgram.vertexColorAttribute);
        this.shaderProgram.jointsAttribute = this.gl.getAttribLocation(this.shaderProgram, "aJoints");
        this.gl.enableVertexAttribArray(this.shaderProgram.jointsAttribute);
        this.shaderProgram.weightsAttribute = this.gl.getAttribLocation(this.shaderProgram, "aWeights");
        this.gl.enableVertexAttribArray(this.shaderProgram.weightsAttribute);
        this.shaderProgram.pMatrixUniform = this.gl.getUniformLocation(this.shaderProgram, "uPMatrix");
        this.shaderProgram.mvMatrixUniform = this.gl.getUniformLocation(this.shaderProgram, "uMVMatrix");
        this.shaderProgram.bonesUniform = this.gl.getUniformLocation(this.shaderProgram, "uBones");
        this.shaderProgram.light_point = this.gl.getUniformLocation(this.shaderProgram, "u_light_point");
        this.shaderProgram.texture = this.gl.getUniformLocation(this.shaderProgram, "u_texture");
        this.shaderProgram.has_texture = this.gl.getUniformLocation(this.shaderProgram, "u_has_texture");
        this.shaderProgram.alpha_cutoff = this.gl.getUniformLocation(this.shaderProgram, "u_alpha_cutoff");
    }

    // Push current matrices to the shader
    setMatrixUniforms() {
        this.gl.uniformMatrix4fv(this.shaderProgram.pMatrixUniform, false, this.pMatrix);
        this.gl.uniformMatrix4fv(this.shaderProgram.mvMatrixUniform, false, this.mvMatrix);
    }

    // Clear the viewport
    clearViewport(){
        this.gl.viewport(0, 0, this.gl.viewportWidth, this.gl.viewportHeight);
        this.gl.clearColor(this.bgColor[0]/255.0, this.bgColor[1]/255.0, this.bgColor[2]/255.0, 1.0);
        this.gl.clear(this.gl.COLOR_BUFFER_BIT | this.gl.DEPTH_BUFFER_BIT);
        
    }

    // Returns the point at which the ray through the centero fthe screen intersects the ground
    getCameraFocus(ground_height){
        // Get a ray through the center of the screen
        let ray = this.getRay([this.gl.viewportWidth*0.5, this.gl.viewportHeight*0.5]) ;
        //intersect it with the ground
        return Renderer.getGroundIntersect(ray, ground_height) ;
    }

    static getGroundIntersect(ray, ground_height){
        let t = (ground_height-ray.p[1])/ray.v[1];
        return [ray.p[0] + ray.v[0]*t, ground_height, ray.p[2] + ray.v[2]*t];
    }

    startRotate(focus, pointer){
        let ray = this.getRay([this.gl.viewportWidth*0.5, this.gl.viewportHeight*0.5]) ;
        if(vec3.distance(this.action_focus, focus) > 1){
            this.action_focus = focus ;
        }

        this.start_position = [ray.p[0], ray.p[1], ray.p[2]] ;
        this.start_pointer = pointer ;
        this.start_camera = mat4.clone(this.mvMatrix);
    }

    continueRotate(pointer){
        var pointer_delta = [pointer.x - this.start_pointer.x, pointer.y - this.start_pointer.y] ;
        var movement = Math.sqrt(pointer_delta[0]*pointer_delta[0]+pointer_delta[1]*pointer_delta[1]);
        if(movement < 1){
            return ;
        }
        let x_axis = [this.start_camera[0], this.start_camera[4], this.start_camera[8]];
        let y_axis = [this.start_camera[1], this.start_camera[5], this.start_camera[9]];
        //axis is 90 degree rotated from mouse move in camera space
        let r = [-pointer_delta[1]*x_axis[0] - pointer_delta[0] * y_axis[0],
                        -pointer_delta[1]*x_axis[1] - pointer_delta[0] * y_axis[1],
                        -pointer_delta[1]*x_axis[2] - pointer_delta[0] * y_axis[2],
                    ];
        let len = Math.sqrt(r[0]*r[0]+r[1]*r[1]+r[2]*r[2]);
        r[0]/=len;
        r[1]/=len;
        r[2]/=len;
        // rotation speed is arbitrarily scaled pointer move distance
        let dtheta = this.rotate_speed * movement;
        let M = mat4.create() ;          
        mat4.rotate(M, M, dtheta, r) ;
        //rotate the position relative to the focus
        let rel_pos = [this.start_position[0]-this.action_focus[0], this.start_position[1]-this.action_focus[1], this.start_position[2]-this.action_focus[2]]; 
        vec3.transformMat4(rel_pos, rel_pos, M) ;
        this.camera_pos = [rel_pos[0] + this.action_focus[0], rel_pos[1] + this.action_focus[1], rel_pos[2] + this.action_focus[2]] ;
        //rotate the up axis, so there's no preferred up direction
        vec3.transformMat4(y_axis, y_axis, M) ;
        mat4.lookAt(this.mvMatrix, this.camera_pos, this.action_focus, y_axis );

        this.start_position = [this.camera_pos[0], this.camera_pos[1], this.camera_pos[2]] ;
        this.start_pointer = pointer ;
        this.start_camera = mat4.clone(this.mvMatrix);

        this.setLightPosition([this.camera_pos[0], this.camera_pos[1] , this.camera_pos[2]]);
    }

    moveCamera(move){
        mat4.translate(this.mvMatrix, this.mvMatrix, [-1*move[0], -1*move[1], -1*move[2]]);
        this.camera_pos[0] += move[0];
        this.camera_pos[1] += move[1];
        this.camera_pos[2] += move[2];
    }

    setZoom(zoom, ground_height){
        if(!ground_height){
            ground_height = this.action_focus[1] ;
        }
        // Send a ray through the center of the screen
        let ray = this.getRay([this.gl.viewportWidth*0.5, this.gl.viewportHeight*0.5]) ;
        //intersect it with the ground then back up by zoom
        let t = (ground_height-ray.p[1])/ray.v[1] - zoom ;
        let position = [ray.p[0] + ray.v[0]*t, ray.p[1] + ray.v[1]*t, ray.p[2] + ray.v[2]*t] ;
        this.moveCamera([position[0]-ray.p[0], position[1]-ray.p[1], position[2]-ray.p[2]]) ;
    }

    getZoom(ground_height){
       // Send a ray through the center of the screen
       let ray = this.getRay([this.gl.viewportWidth*0.5, this.gl.viewportHeight*0.5]) ;
       //intersect it with the ground
       let t = (ground_height-ray.p[1])/ray.v[1] ;
       return t ;
    }    

    // Sets light position
    //TODO very shader specific, not a fan, needed it to migrate old code
    setLightPosition(light_point){
        this.gl.uniform3fv(this.shaderProgram.light_point, light_point);
        this.light_point = light_point ;
    }

    drawMeshes(){
        if(!this.xr_session){
            this.setMatrixUniforms();
        }
        for(let id in this.buffers){
            this.drawModel(this.buffers[id]);
        }
    }

    finishFrame(){
        this.gl.finish();
    }

    // Given a 3D point return the point on the canvas it would be on
    projectToScreen(point){
        let p = vec4.fromValues(point[0], point[1], point[2], 1);
        let np = [0,0,0] ;
        vec4.transformMat4(np, p, this.mvMatrix);
        vec4.transformMat4(np, np, this.pMatrix);
        if(np[3] < 0){ // behind camera
            return null;
        }else{
            np[0]/=np[3];
            np[1]/=np[3];
            np[2]/=np[3];
            return [(np[0]+1) * this.gl.viewportWidth * 0.5, (-np[1]+1) * this.gl.viewportHeight * 0.5];
        }
    }

    // Takes a pixel position on the screen and creates a 3D ray from the viewpoint toward that pixel in voxel space.
    // returns an object with p and v both float32 arrays with 3 elements (compatible with CPP API raytrace)
    getRay(screen_pos){
        return Renderer.getPixelRay(this.camera_pos, screen_pos, this.pMatrix, this.mvMatrix, this.gl);
    }
    static getPixelRay(camera_pos, screen_pos, pMatrix, mvMatrix, gl){
        // recreate the transformation used by the viewing pipleline
        var M  = mat4.create();
        mat4.multiply(M, pMatrix, mvMatrix ) ;
        mat4.invert(M,M); // invert it

        let pos = new Float32Array([camera_pos[0], camera_pos[1], camera_pos[2]]);

        // Get the pixel vector in screen space using viewport parameters.
        var p = vec4.fromValues(2*screen_pos[0]/gl.viewportWidth-1, -1*(2*screen_pos[1]/gl.viewportHeight-1), 1, 1);
        vec4.transformMat4(p, p, M);
        let v = new Float32Array([p[0]/p[3]-pos[0], p[1]/p[3]-pos[1], p[2]/p[3]-pos[2]]);
        
        var n = Math.sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
        v[0]/=n;
        v[1]/=n; // Normalize the direction.
        v[2]/=n;

        return {p:pos,v:v};
    }

    clearBuffers(){
        this.buffers = {} ;
        //TODO delete textures?
    }

    // Binds a webGl buffer to the buffer data provided and puts it in buffers[id]
    prepareBuffer(id, buffer_data){
        if(!(id in this.buffers)){ // New buffer
            this.buffers[id] = {};
            this.buffers[id].position = this.gl.createBuffer();
            this.buffers[id].color = this.gl.createBuffer();
            this.buffers[id].normal = this.gl.createBuffer();
            this.buffers[id].tex_coord = this.gl.createBuffer();
            this.buffers[id].texture_id = -1;
            this.buffers[id].joints = this.gl.createBuffer();
            this.buffers[id].weights= this.gl.createBuffer();
        }
        let num_vertices = buffer_data.vertices ;
        if(num_vertices == 0){
            this.buffers[id].ready = false;
            return ;
        }
        if(buffer_data.position){
            this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.buffers[id].position );
            this.gl.bufferData(this.gl.ARRAY_BUFFER, buffer_data.position, this.gl.STATIC_DRAW);
            this.buffers[id].position.itemSize = 3;
            this.buffers[id].position.numItems = num_vertices;
        }

        if(buffer_data.color){
            this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.buffers[id].color );
            this.gl.bufferData(this.gl.ARRAY_BUFFER, buffer_data.color, this.gl.STATIC_DRAW);
            this.buffers[id].color.itemSize = 3;
            this.buffers[id].color.numItems = num_vertices;
        }

        if(buffer_data.normal){
            this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.buffers[id].normal );
            this.gl.bufferData(this.gl.ARRAY_BUFFER, buffer_data.normal, this.gl.STATIC_DRAW);
            this.buffers[id].normal.itemSize = 3;
            this.buffers[id].normal.numItems = num_vertices;
        }

        if(buffer_data.tex_coord){
            this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.buffers[id].tex_coord );
            this.gl.bufferData(this.gl.ARRAY_BUFFER, buffer_data.tex_coord, this.gl.STATIC_DRAW);
            this.buffers[id].tex_coord.itemSize = 2;
            this.buffers[id].tex_coord.numItems = num_vertices;
        }

        if(buffer_data.weights){
            this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.buffers[id].weights );
            this.gl.bufferData(this.gl.ARRAY_BUFFER, buffer_data.weights, this.gl.STATIC_DRAW);
            this.buffers[id].weights.itemSize = 4;
            this.buffers[id].weights.numItems = num_vertices;
        }

        if(buffer_data.joints){
            this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.buffers[id].joints );
            this.gl.bufferData(this.gl.ARRAY_BUFFER, buffer_data.joints, this.gl.STATIC_DRAW);
            this.buffers[id].joints.itemSize = 4;
            this.buffers[id].joints.numItems = num_vertices;
        }

        if(buffer_data.material){
            //console.log("Javascript preparebuffer got material:\n");
            let mat = buffer_data.material ;
            //console.log(mat);
            if(mat.has_texture == 1){
                //console.log("got texture image!");
                this.buffers[id].texture = this.gl.createTexture();
                this.gl.bindTexture(this.gl.TEXTURE_2D, this.buffers[id].texture);
                this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_WRAP_S, this.gl.CLAMP_TO_EDGE);
                this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_WRAP_T, this.gl.CLAMP_TO_EDGE);
                this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MIN_FILTER, this.gl.LINEAR);
                this.gl.texParameteri(this.gl.TEXTURE_2D, this.gl.TEXTURE_MAG_FILTER, this.gl.LINEAR);

                
                //console.log(mat.image_width +", " + mat.image_height +", " + mat.image_channels);
                if(mat.image_channels == 3){
                    this.gl.texImage2D(this.gl.TEXTURE_2D, 0, this.gl.RGB, 
                        mat.image_width, mat.image_height, 
                        0, this.gl.RGB, this.gl.UNSIGNED_BYTE, new Uint8Array(mat.image_data));
                }else if(mat.image_channels == 4){
                    this.gl.texImage2D(this.gl.TEXTURE_2D, 0, this.gl.RGBA, 
                        mat.image_width, mat.image_height, 
                        0, this.gl.RGBA, this.gl.UNSIGNED_BYTE, new Uint8Array(mat.image_data));
                }


                //console.log(buffer_data.materials);
                this.buffers[id].texture_id = parseInt(id) +2; // TODO would conflict if more than one gltf at a time

                //console.log("binding Texture id: " + this.buffers[id].texture_id + " \n");
                //console.log(this.buffers[id].texture);
                this.gl.activeTexture(this.gl.TEXTURE0 + this.buffers[id].texture_id );
                this.gl.bindTexture(this.gl.TEXTURE_2D, this.buffers[id].texture );
                this.gl.uniform1i(this.shaderProgram.texture, this.buffers[id].texture_id );
                
            }

        }

        if(buffer_data.bones){
            this.gl.uniformMatrix4fv(this.shaderProgram.uBones, false, buffer_data.bones);
        }

        this.buffers[id].ready = true;
    }

    drawModel(buffer){
        if(buffer.ready){
            let position_buffer = buffer.position;
            let color_buffer = buffer.color;
            let normal_buffer = buffer.normal;
            let tex_coord_buffer = buffer.tex_coord;
            let joints_buffer = buffer.joints;
            let weights_buffer = buffer.weights;
            this.gl.bindBuffer(this.gl.ARRAY_BUFFER, position_buffer);
            this.gl.vertexAttribPointer(this.shaderProgram.vertexPositionAttribute, position_buffer.itemSize, this.gl.FLOAT, false, 0, 0);
            this.gl.bindBuffer(this.gl.ARRAY_BUFFER, normal_buffer);
            this.gl.vertexAttribPointer(this.shaderProgram.vertexNormalAttribute, normal_buffer.itemSize, this.gl.FLOAT, false, 0, 0);
            this.gl.bindBuffer(this.gl.ARRAY_BUFFER, tex_coord_buffer);
            this.gl.vertexAttribPointer(this.shaderProgram.vertexTexcoordAttribute, tex_coord_buffer.itemSize, this.gl.FLOAT, false, 0, 0);
            this.gl.bindBuffer(this.gl.ARRAY_BUFFER, color_buffer);
            this.gl.vertexAttribPointer(this.shaderProgram.vertexColorAttribute, color_buffer.itemSize, this.gl.FLOAT, false, 0, 0);
            this.gl.bindBuffer(this.gl.ARRAY_BUFFER, joints_buffer);
            this.gl.vertexAttribPointer(this.shaderProgram.jointsAttribute, joints_buffer.itemSize, this.gl.INT, false, 0, 0);
            this.gl.bindBuffer(this.gl.ARRAY_BUFFER, weights_buffer);
            this.gl.vertexAttribPointer(this.shaderProgram.weightsAttribute, weights_buffer.itemSize, this.gl.FLOAT, false, 0, 0);

            if(buffer.texture_id >= 0){
                //console.log("Drawing texture for index " + (buffer.texture_id+2) +"\n");
                this.gl.activeTexture(this.gl.TEXTURE0 + buffer.texture_id );
                this.gl.bindTexture(this.gl.TEXTURE_2D, buffer.texture );
                this.gl.uniform1i(this.shaderProgram.texture, buffer.texture_id );

                this.gl.uniform1i(this.shaderProgram.has_texture, 1 );
            }else{
                //console.log("No texture for index " + (buffer.texture_id+2) +"\n");
                this.gl.uniform1i(this.shaderProgram.has_texture, 0 );
            }

            this.gl.uniform1f(this.shaderProgram.alpha_cutoff, 0.5 );

            this.gl.drawArrays(this.gl.TRIANGLES, 0, position_buffer.numItems);
        }
    }

    startXRSession(){
        navigator.xr.requestSession('immersive-vr').then(renderer.onXRSessionStarted);
    }

    onXRSessionStarted(session){
        renderer.xr_session = session;
        //xrButton.textContent = 'Exit VR';

        // Listen for the sessions 'end' event so we can respond if the user
        // or UA ends the session for any reason.
        session.addEventListener('end', renderer.onXRSessionEnded);

        // Create a WebGL context to render with, initialized to be compatible
        // with the XRDisplay we're presenting to.
        //let canvas = document.createElement('canvas');
        //gl = canvas.getContext('webgl2', { xrCompatible: true });

        // Use the new WebGL context to create a XRWebGLLayer and set it as the
        // sessions baseLayer. This allows any content rendered to the layer to
        // be displayed on the XRDevice.
        session.updateRenderState({ baseLayer: new XRWebGLLayer(session, renderer.gl) });

        // Initialize the shaders
        //initShaderProgram(gl, "shader-fs", "shader-vs");

        // Get a reference space, which is required for querying poses. In this
        // case an 'local' reference space means that all poses will be relative
        // to the location where the XRDevice was first detected.
        session.requestReferenceSpace('local').then((refSpace) => {
            renderer.xr_ref_space = refSpace;

            // Inform the session that we're ready to begin drawing.
            session.requestAnimationFrame(renderer.onXRFrame);
        });
    }

      // Called either when the user has explicitly ended the session by calling
      // session.end() or when the UA has ended the session for any reason.
      // At this point the session object is no longer usable and should be
      // discarded.
    onXRSessionEnded(event) {
        renderer.xr_session = null;
        console.log("XR session ended.");
    }

      // Called every time the XRSession requests that a new frame be drawn.
    onXRFrame(time, frame) {

        window.setTimeout(tools.renderer.animate,1);

         // console.log(time);
        let session = frame.session;

        // Inform the session that we're ready for the next frame.
        session.requestAnimationFrame(renderer.onXRFrame);

        // Get the XRDevice pose relative to the reference space we created
        // earlier.
        let pose = frame.getViewerPose(renderer.xr_ref_space);

        // Getting the pose may fail if, for example, tracking is lost. So we
        // have to check to make sure that we got a valid pose before attempting
        // to render with it. If not in this case we'll just leave the
        // framebuffer cleared, so tracking loss means the scene will simply
        // disappear.
        if (pose) {
            let glLayer = session.renderState.baseLayer;

            // If we do have a valid pose, bind the WebGL layer's framebuffer,
            // which is where any content to be displayed on the XRDevice must be
            // rendered.
            renderer.gl.bindFramebuffer(renderer.gl.FRAMEBUFFER, glLayer.framebuffer);

            // Clear the framebuffer
            renderer.gl.clearColor(renderer.bgColor[0]/255.0, renderer.bgColor[1]/255.0, renderer.bgColor[2]/255.0, 1.0);
            renderer.gl.enable(renderer.gl.DEPTH_TEST);
            renderer.gl.enable(renderer.gl.CULL_FACE);
            renderer.gl.cullFace(renderer.gl.BACK);

            renderer.gl.clear(renderer.gl.COLOR_BUFFER_BIT | renderer.gl.DEPTH_BUFFER_BIT);
            

            if(!renderer.model_pose){
                renderer.model_pose = mat4.create();
                mat4.identity(renderer.model_pose);
                mat4.translate(renderer.model_pose, renderer.model_pose,[0,0,-0.5]);
            }
            
            let any_grab = false;
            
            for (let inputSource of session.inputSources) {
                let targetRayPose = frame.getPose(inputSource.targetRaySpace, renderer.xr_ref_space);
                if(targetRayPose && inputSource.gripSpace){
                    //console.log(inputSource);
                    let grabbing = false; 
                    if(inputSource.gamepad){
                        for(let button of inputSource.gamepad.buttons){
                            grabbing = grabbing || button.pressed;
                            //console.log(button);
                        }
                    }
                    any_grab = any_grab || grabbing ;
                    
                    let grip_pose = frame.getPose(inputSource.gripSpace, renderer.xr_ref_space).transform.matrix;
                    // start of grab, fetch starting poses
                    if(grabbing && !renderer.grab_pose){
                        //console.log(inputSource);
                        renderer.grab_pose = mat4.create();
                        renderer.grab_pose.set(grip_pose);
                        renderer.grab_model_pose = mat4.create();
                        renderer.grab_model_pose.set(renderer.model_pose) ;
                        //console.log("grabbed:");
                        //console.log(renderer.grab_pose);
                    }
                    
                    if(grabbing){
                        //console.log("gripping:");
                        //console.log(grip_pose);
                        let MP = mat4.create();
                        mat4.invert(MP, renderer.grab_pose); // TODO cache at grab time
                        mat4.multiply(MP,grip_pose, MP);
                        mat4.multiply(renderer.model_pose, MP, renderer.grab_model_pose);
                        //console.log(renderer.model_pose);
                        break ; // don't check next controllers
                    }
                    
                }
            }

            if(!any_grab){// stopped grabbing, clear saved poses
                renderer.grab_pose = null;
                renderer.grab_model_pose = null;
            }

            // undo the model transformation for the light point so it doesn't move with the model
            let lp = vec4.fromValues(renderer.light_point[0], renderer.light_point[1], renderer.light_point[2], 1);
            let L = mat4.create();
            mat4.invert(L, renderer.model_pose);
            vec4.transformMat4(lp, lp, L);
			renderer.gl.uniform3fv(renderer.shaderProgram.light_point, [lp[0], lp[1], lp[2]]);
            //console.log(lp);
            

            for (let view of pose.views) {
                let viewport = glLayer.getViewport(view);
                renderer.gl.viewport(viewport.x, viewport.y,
                            viewport.width, viewport.height);
                //console.log("View matrix:");
                //console.log(view.transform.inverse.matrix);

                // Draw a scene using view.projectionMatrix as the projection matrix
                // and view.transform to position the virtual camera. If you need a
                // view matrix, use view.transform.inverse.matrix.

                renderer.gl.uniformMatrix4fv(renderer.shaderProgram.pMatrixUniform, false, view.projectionMatrix);

                let M = mat4.create();
                mat4.multiply(M,view.transform.inverse.matrix, renderer.model_pose );
                renderer.gl.uniformMatrix4fv(renderer.shaderProgram.mvMatrixUniform, false, M);
                //drawModel(gl, model);
                renderer.drawMeshes();
                
                for (let inputSource of session.inputSources) {
                    let targetRayPose = frame.getPose(inputSource.targetRaySpace, renderer.xr_ref_space);
                    if(targetRayPose && inputSource.gripSpace){
                        let gripPose = frame.getPose(inputSource.gripSpace, renderer.xr_ref_space);
                        if (gripPose) {
                            let G = mat4.create();
                            mat4.multiply(G,view.transform.inverse.matrix, gripPose.transform.matrix );
                            renderer.gl.uniformMatrix4fv(renderer.shaderProgram.mvMatrixUniform, false, G);
                            //drawModel(gl, grip_cursor);
                        }
                    }
                }
                
            }

        }

    }

    animating = false;
    animate(){
        if(!this.animating){
            this.animating = true;

            let new_buffer_data = tools.API.call("getUpdatedBuffers", null, new Serializer());
            if(new_buffer_data && Object.keys(new_buffer_data).length > 0 && "material" in new_buffer_data[0]){
                tools.renderer.clearBuffers();
            }
            for(let id in new_buffer_data){
                tools.renderer.prepareBuffer(id, new_buffer_data[id]);
            }

            this.animating = false;
        }
    }
    


}