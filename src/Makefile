all:
	emcc cgol.c -o cgol.wasm \
		-O3 \
		-Wall -Wextra -Werror \
		-s STANDALONE_WASM=1 \
		-s EXPORTED_FUNCTIONS='["_get_state_data", "_get_state_width", "_get_state_height", "_resize_state", "_update_state", "_malloc","_free"]' \
		-s EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]' \
		-s EXPORT_ES6=0 \
		-s ENVIRONMENT=web \
		--no-entry

