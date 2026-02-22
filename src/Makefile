all:
	emcc cgol.c -O3 -o cgol.wasm \
		-s STANDALONE_WASM \
		-s EXPORTED_FUNCTIONS='["_get_state_data", "_get_state_width", "_get_state_height", "_init_state", "_destroy_state", "_update_state", "_malloc","_free"]' \
		-s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
		-s EXPORT_ES6=0 \
		-s ENVIRONMENT=web \
		--no-entry
	mv cgol.wasm ../docs