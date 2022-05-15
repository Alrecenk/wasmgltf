class PoseMode extends ExecutionMode{
    /* Input trackers. */

    // Mouse position
	mouse_x;
	mouse_y;
	// Where the mouse was last pressed down.
	mouse_down_x;
	mouse_down_y;
	//Whether the mouse is currently down.
	mouse_down = false ;
	//Which button is down (0 is left, 2 is right)
    mouse_button ;

    dragging = false;
    held_id ; // currently held box id
    
    rotating = false;
    camera_zoom;
    camera_focus = [0,0,0];

    axis_total = 0 ;

    model_pose = null;

    hand_pose = [mat4.create(),mat4.create()] ;

    // Tools is an object with string keys that may include things such as the canvas,
    // API WASM Module, an Interface manager, and/or a mesh manager for shared webGL functionality
    constructor(tools){
        super(tools) ;
        this.model_pose = mat4.create();
        mat4.identity(this.model_pose);

        mat4.translate(this.hand_pose[0], this.model_pose,[2.5,1.5,0]);
        mat4.translate(this.hand_pose[1], this.model_pose,[-2.5,1.5,0]);
        
    }

    // Called when the mode is becoming active (previous mode will have already exited)
    enter(previous_mode){
        this.camera_zoom = tools.renderer.getZoom(0); 
    }

    // Called when a mode is becoming inactive (prior to calling to enter on that mode)
    exit(next_mode){
    }

    // Called at regular intervals by the main app when the mode is active (30 or 60fps probably but not guarsnteed)
    timer(){

    }

    // Called when the app should be redrawn
    // Note: the elements to draw onto or with should be included in the tools on construction and saved for the duration of the mode
    drawFrame(frame_id){
        // Get any mesh updates pending in the module
        if(frame_id == 0){
            let new_buffer_data = tools.API.call("getUpdatedBuffers", null, new Serializer());
            for(let id in new_buffer_data){
                tools.renderer.prepareBuffer(id, new_buffer_data[id]);
            }
        }
        // Draw the models
        tools.renderer.drawMesh("MAIN", this.model_pose);
        tools.renderer.drawMesh("HAND", this.hand_pose[0]);
        tools.renderer.drawMesh("HAND", this.hand_pose[1]);

    }


    pointerDown(pointers){
        if(this.mouse_down){ // ignore second mouse click while dragging
			return ;
		}
		this.mouse_down_x = pointers[0].x;
		this.mouse_down_y = pointers[0].y;
		this.mouse_down = true ;
		this.mouse_button = pointers[0].button ;
        
        if(this.mouse_button == 0){
            this.rotating = true;
            this.tools.renderer.startRotate(this.camera_focus, pointers[0]);
        }else{

            let ray = tools.renderer.getRay([this.mouse_down_x,this.mouse_down_y]);
            ray.name = "drag";
            tools.API.call("createPin", ray, new Serializer()); 
            this.dragging = true;
        }
    }

	pointerMove(pointers){
		this.mouse_x = pointers[0].x;
		this.mouse_y = pointers[0].y;
		if(this.mouse_down){
            if(this.dragging){
                let ray = tools.renderer.getRay([this.mouse_x,this.mouse_y]);
                ray.name = "drag";
                tools.API.call("setPinTarget", ray, new Serializer()); 

            }else if(this.rotating){
               this.tools.renderer.continueRotate(pointers[0]);
            }
		}
    }

    pointerUp(pointers){ // Note only pointers still down
        if(!this.mouse_down){
			return ;
        }
        if(this.dragging){
            let params = {name:"drag"} ;
            tools.API.call("deletePin", params, new Serializer()); 
        }
        this.mouse_down = false ;
        this.dragging = false;
        this.rotating = false;
    }

    mouseWheelListener(event){
        // Firefox and Chrome get different numbers from different events. One is 3 the other is -120 per notch.
        var scroll = event.wheelDelta ? -event.wheelDelta*.2 : event.detail*8; 
        // Adjust camera zoom by mouse wheel scroll.
		this.camera_zoom *= Math.pow(1.005,scroll);
        tools.renderer.setZoom(this.camera_zoom) ;
    }

	keyDownListener(event){
	    var key_code = event.keyCode || event.which;
        var character = String.fromCharCode(key_code); // This only works with letters. Use key-code directly for others.
        console.log(character);
        if(character == 'M'){
            console.log("testing memory:");
            let params = {};
                params.size = 100000000;
                params.num = 1 ; 
                let ptr = tools.API.call("testAllocate", params, new Serializer()).ptr;
                console.log(ptr);

        }
    }

	keyUpListener(event){

    }
    
    vrInputSourcesUpdated(input_sources, frame){
        let which_hand = 0 ;
        let GRIP = 2 ;
        let TRIGGER = 1 ;
        let pose_hand = -1 ;
        let grab_hand = -1 ;
        for (let inputSource of input_sources) {
            let targetRayPose = frame.getPose(inputSource.targetRaySpace, tools.renderer.xr_ref_space);

            if(targetRayPose && inputSource.gripSpace){
                if(inputSource.gamepad){                    
                    let which_button = 0 ;
                    for(let button of inputSource.gamepad.buttons){
                        which_button++;
                        if(button.pressed){
                            //console.log("button pressed " + which_button);
                            if(which_button == GRIP && grab_hand < 0){
                                grab_hand = which_hand;
                            }
                            if(which_button == TRIGGER && pose_hand < 0){
                                pose_hand = which_hand ;
                            }
                        }
                    }
                    

                    for(let axis of inputSource.gamepad.axes){
                        this.axis_total += axis ;
                    }
                }
                
                
                let grip_pose = frame.getPose(inputSource.gripSpace, tools.renderer.xr_ref_space).transform.matrix;
                if(which_hand != pose_hand){ // TODO remove freeze hand to make sure button press registers
                    this.hand_pose[which_hand] = grip_pose;
                }
                

                if(grab_hand == which_hand){
                    if(!this.grab_pose){ // start of grab, fetch starting poses
                        this.grab_pose = mat4.create();
                        this.grab_pose.set(grip_pose);
                        this.grab_model_pose = mat4.create();
                        this.grab_model_pose.set(this.model_pose) ;
                        this.grab_axis_total = this.axis_total ;
                    }

                    let MP = mat4.create();
                    let inv = mat4.create();
                    mat4.invert(inv, this.grab_pose); // TODO cache at grab time
                    mat4.multiply(MP,inv, MP);
                    mat4.multiply(MP,grip_pose, MP);
                    let scale = Math.pow(1.05, (this.axis_total - this.grab_axis_total)*0.3);
                    mat4.scale(MP, MP,[scale,scale,scale]);
                    mat4.multiply(this.model_pose, MP, this.grab_model_pose);

                }
                
            }
            which_hand++;
        }


        if(grab_hand < 0){// stopped grabbing, clear saved poses
            this.grab_pose = null;
            this.grab_model_pose = null;
        }
    }
}