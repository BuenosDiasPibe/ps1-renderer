
vim.o.makeprg = './nob'
vim.opt.path:append 'vendor'
vim.opt.path:append 'vendor/cimgui'
vim.api.nvim_create_autocmd('FileType', {
	pattern = {"c"},
	callback = function(args)
		vim.b.man_default_sections = '3G,3,2'
		vim.bo.makeprg = './nob'
	end,
})
