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

    // Tools is an object with string keys that may include things such as the canvas,
    // API WASM Module, an Interface manager, and/or a mesh manager for shared webGL functionality
    constructor(tools){
        super(tools) ;

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
            if(new_buffer_data && Object.keys(new_buffer_data).length > 0 && "material" in new_buffer_data[0]){
                tools.renderer.clearBuffers();
            }
            for(let id in new_buffer_data){
                tools.renderer.prepareBuffer(id, new_buffer_data[id]);
            }
        }
        // Draw the models
        tools.renderer.drawMeshes();
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
        let any_grab = false;
        for (let inputSource of input_sources) {
            let targetRayPose = frame.getPose(inputSource.targetRaySpace, tools.renderer.xr_ref_space);
            if(targetRayPose && inputSource.gripSpace){
                let grabbing = false; 
                if(inputSource.gamepad){
                    for(let button of inputSource.gamepad.buttons){
                        grabbing = grabbing || button.pressed;
                        //console.log(button);
                    }
                    
                    for(let axis of inputSource.gamepad.axes){
                        this.axis_total += axis ;
                    }
                }
                any_grab = any_grab || grabbing ;
                
                let grip_pose = frame.getPose(inputSource.gripSpace, tools.renderer.xr_ref_space).transform.matrix;
                // start of grab, fetch starting poses
                if(grabbing && !this.grab_pose){
                    //console.log(inputSource);
                    this.grab_pose = mat4.create();
                    this.grab_pose.set(grip_pose);
                    this.grab_model_pose = mat4.create();
                    this.grab_model_pose.set(tools.renderer.model_pose) ;
                    this.grab_axis_total = this.axis_total ;
                    //console.log("grabbed:");
                    //console.log(renderer.grab_pose);
                }
                
                if(grabbing){
                    //console.log("gripping:");
                    //console.log(grip_pose);


                    let MP = mat4.create();

                    

                    let inv = mat4.create();
                    mat4.invert(inv, this.grab_pose); // TODO cache at grab time

                    mat4.multiply(MP,inv, MP);

                    mat4.multiply(MP,grip_pose, MP);

                    let scale = Math.pow(1.05, (this.axis_total - this.grab_axis_total)*0.3);
                    mat4.scale(MP, MP,[scale,scale,scale]);
                    
                    
                    mat4.multiply(tools.renderer.model_pose, MP, this.grab_model_pose);

                    //console.log(renderer.model_pose);
                    break ; // don't check next controllers
                }
                
            }
        }
        if(!any_grab){// stopped grabbing, clear saved poses
            this.grab_pose = null;
            this.grab_model_pose = null;
        }
    }
}